#!/usr/bin/env bash
set -uo pipefail

# ============================================================================
#  PRBot - Inside Container Script (Multi-Repo Support)
#  Uses repo tool for multi-repo clone, PR refs for checkout
# ============================================================================

# Source shared functions from common_utils.sh
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../jenkins/common_utils.sh"

# ============================================================================
#  Required Environment Variables (UPPERCASE standardized names)
# ============================================================================
: "${BB_TOKEN:?BB_TOKEN must be set}"
: "${TARGET_REPO:?TARGET_REPO must be set}"
: "${PR_ID:?PR_ID must be set}"
: "${DEVICE_LIST:?DEVICE_LIST must be set}"
: "${RESULTS_HTML_URL:?RESULTS_HTML_URL must be set}"
: "${SDK_ROOT:?SDK_ROOT must be set}"

# Manifest configuration (can be derived or passed)
: "${MANIFEST_URL:=ssh://git@bitbucket.itg.ti.com/processor-sdk/manifests.git}"
MANIFEST_TEAM="${TARGET_REPO##*/}"  # Extract repo name for manifest team
: "${MANIFEST_FILE:=dev.xml}"

# Bitbucket API base URLs
: "${BASE_API:=https://bitbucket.itg.ti.com/rest/api/1.0}"
: "${BASE_INSIGHTS:=https://bitbucket.itg.ti.com/rest/insights/1.0}"

# Derived from manifest (set by clone_with_repo_tool in common_utils.sh)
BB_PROJECT=""
BB_REPO=""
TARGET_REPO_PATH=""

# Optional
AI_REPORT="${AI_REPORT:-false}"
AI_RESULTS_HTML_URL="${AI_RESULTS_HTML_URL:-}"
: "${PR_DESCRIPTION:=}"

# Working directory (mounted from host)
WORK_DIR="$(pwd)"

# Artifacts directory - use env var if set (passed from docker_run.sh), otherwise default
: "${ARTIFACTS_DIR:=${WORK_DIR}/artifacts}"
LOG_DIR="${ARTIFACTS_DIR}/logs"

# Commit limits
MAX_COMMIT_LIMIT=3
WARNING_COMMIT_LIMIT=1

# Results tracking
RESULTS=()
ALL_PASS=1

# ============================================================================
#  PRBot-Specific Helper Functions
# ============================================================================

add_emoji() {
    local text="$1"
    text="${text//PASS:/✅}"
    text="${text//FAIL:/❌}"
    text="${text//WARN:/⚠️}"
    text="${text//INFO:/ℹ️}"
    echo "$text"
}

# ============================================================================
#  Git Checks (runs only on TARGET_REPO)
# ============================================================================

check_commit_message() {
    local msg summary description trailers parts n fix
    local COMMIT_MESSAGE=0

    read -r -d '' msg < <( git log -1 --pretty=format:%B && printf '\0' )

    mapfile -t parts < <(
        printf '%s' "$msg" |
            awk -v RS='' '{ gsub(/\r?\n[[:space:]]*\r?\n+/, "\n\n"); print }'
    )
    n=${#parts[@]}
    summary=${parts[0]}
    description=""
    (( n > 1 )) && description=${parts[1]}
    trailers=""
    (( n > 2 )) && trailers=$(printf '%s\n\n' "${parts[@]:2}")

    fix=$(grep -E '^[[:space:]]*Fixes:[[:space:]].+' <<<"$trailers" || true)

    if (( n < 3 )); then
        RESULTS+=("FAIL: Must have SUMMARY, Fixes:, Signed-off-by: paragraphs")
        COMMIT_MESSAGE=1
    else
        echo "PASS: Found ${n} paragraphs"
    fi

    local len=${#summary}
    if (( len > 80 )); then
        RESULTS+=("WARN: Summary too long (${len} > 80)")
    else
        echo "PASS: Summary length OK (${len})"
    fi

    if [[ $summary =~ ^[a-z0-9/]+:[[:space:]].+:[[:space:]].+ ]]; then
        echo "PASS: Summary matches '<subcomp>: <verb>: <desc>'"
    else
        echo "FAIL: Summary must match '<subcomp>: <verb>: <desc>'"
        RESULTS+=("FAIL: Summary must match '<subcomp>: <verb>: <desc>'")
        COMMIT_MESSAGE=1
    fi

    local after="${summary##*: }"
    local fw_sum="${after%% *}"
    if [[ $fw_sum =~ (ed|es|ing)$ ]]; then
        RESULTS+=("FAIL: Summary verb ends with ed/es/ing")
        COMMIT_MESSAGE=1
    else
        echo "PASS: Summary verb '$fw_sum' looks imperative"
    fi

    if [[ -n $description ]]; then
        local desc_line="${description#[-*][[:space:]]}"
        local fw_desc="${desc_line%% *}"
        if [[ $fw_desc =~ (ed|es|ing)$ ]]; then
            RESULTS+=("INFO: Description verb ends with ed/es/ing")
        else
            echo "PASS: Description verb '$fw_desc' looks imperative"
        fi
    else
        RESULTS+=("INFO: No description to check")
    fi

    if grep -Eq '^[[:space:]]*Fixes:[[:space:]][A-Z0-9_-]+(-[0-9]+)?(,\s*[A-Z0-9_-]+(-[0-9]+)?)*$' <<<"$fix"; then
        echo "PASS: Fixes line OK"
    else
        RESULTS+=("INFO: Fixes line missing or malformed")
    fi

    if grep -Eq '^Signed-off-by:[[:space:]].+<[^@]+@[^>]+>$' <<<"$trailers"; then
        echo "PASS: Signed-off-by line OK"
    else
        RESULTS+=("FAIL: Signed-off-by line missing or malformed")
        COMMIT_MESSAGE=1
    fi

    if [[ -n $description ]]; then
        local bad=0
        while IFS= read -r line; do
            line="${line#[-*][[:space:]]}"
            if (( ${#line} > 80 )); then
                RESULTS+=("FAIL: Body line too long (${#line} chars)")
                COMMIT_MESSAGE=1
                bad=1
                break
            fi
        done <<<"$description"
        (( bad == 0 )) && echo "PASS: All body lines <=80 chars"
    else
        RESULTS+=("INFO: No body lines to check")
    fi

    if (( COMMIT_MESSAGE )); then
        echo "WARN: Existing issues to be resolved"
    else
        RESULTS+=("PASS: Commit message correct")
    fi

    return $COMMIT_MESSAGE
}

# Fetch main branch with enough history for comparison
# Called once before rebase and commit count checks
# Fixes: shallow clone (--depth=1) doesn't have enough history for merge-base
fetch_main_for_comparison() {
    log "Fetching main branch for comparison..."

    # Unshallow if needed to get full history for comparison
    if git rev-parse --is-shallow-repository | grep -q true; then
        log "Repository is shallow, fetching with depth for comparison..."
        # Fetch main with enough depth to find merge-base (50 commits should be plenty)
        git fetch --depth=50 origin main:refs/remotes/origin/main 2>/dev/null || \
        git fetch --depth=50 origin master:refs/remotes/origin/master 2>/dev/null || true
    else
        git fetch origin main:refs/remotes/origin/main 2>/dev/null || \
        git fetch origin master:refs/remotes/origin/master 2>/dev/null || true
    fi
}

# Get the main branch ref (main or master)
get_main_ref() {
    if git rev-parse "refs/remotes/origin/main" &>/dev/null; then
        echo "refs/remotes/origin/main"
    elif git rev-parse "refs/remotes/origin/master" &>/dev/null; then
        echo "refs/remotes/origin/master"
    else
        echo ""
    fi
}

check_rebase() {
    log "Checking rebase onto main..."

    local main_ref
    main_ref=$(get_main_ref)

    if [[ -z "$main_ref" ]]; then
        RESULTS+=("WARN: Could not find main/master branch")
        return
    fi

    local upstream
    upstream=$(git merge-base HEAD "$main_ref" 2>/dev/null || echo "")

    if [[ -z "$upstream" ]]; then
        RESULTS+=("WARN: Could not determine rebase status")
        return
    fi

    if [[ "$upstream" == "$(git rev-parse "$main_ref")" ]]; then
        RESULTS+=("PASS: Branch is rebased onto main")
    else
        RESULTS+=("FAIL: Branch is NOT rebased onto main")
        ALL_PASS=0
    fi
}

check_commit_count() {
    log "Checking number of incoming commits..."

    local main_ref
    main_ref=$(get_main_ref)

    if [[ -z "$main_ref" ]]; then
        RESULTS+=("WARN: Could not determine commit count (no main branch)")
        return
    fi

    # Use merge-base to find common ancestor, then count commits from there
    local merge_base
    merge_base=$(git merge-base HEAD "$main_ref" 2>/dev/null || echo "")

    local incoming_commits
    if [[ -n "$merge_base" ]]; then
        # Count commits from merge-base to HEAD (PR commits only)
        incoming_commits=$(git rev-list --count "${merge_base}..HEAD" 2>/dev/null || echo "0")
    else
        # Fallback: can't find merge-base, use direct comparison
        incoming_commits=$(git log "$main_ref"..HEAD --oneline 2>/dev/null | wc -l || echo "0")
    fi

    if (( incoming_commits <= WARNING_COMMIT_LIMIT )); then
        RESULTS+=("PASS: Commit count of ${incoming_commits} within the Recommended Range")
    elif (( incoming_commits <= MAX_COMMIT_LIMIT )); then
        RESULTS+=("WARN: Commit count of ${incoming_commits} is in the Warning Range, Recommended Squash before Proceeding")
    else
        RESULTS+=("FAIL: ${incoming_commits} commits incoming from the current branch, Limit is ${MAX_COMMIT_LIMIT}, Please Squash before Proceeding")
        ALL_PASS=0
    fi
}

run_git_checks() {
    log "========== Running Git Checks on $TARGET_REPO =========="
    cd "$WORK_DIR/$TARGET_REPO_PATH"
    log "Working in: $(pwd)"

    # Fetch main with sufficient history for comparisons
    fetch_main_for_comparison

    check_rebase
    check_commit_count
    log "Checking commit message..."
    check_commit_message || ALL_PASS=0

    log "Git checks completed"
}

# ============================================================================
#  Publish Git Checks Report
# ============================================================================

publish_git_checks_report() {
    log "Publishing Git Checks report to Bitbucket..."

    cd "$WORK_DIR/$TARGET_REPO_PATH"
    local COMMIT_ID
    COMMIT_ID=$(git rev-parse HEAD)

    local REPORT_KEY="git_checks"
    local REPORT_URL="$BASE_INSIGHTS/projects/$BB_PROJECT/repos/$BB_REPO/commits/$COMMIT_ID/reports/$REPORT_KEY"

    local DETAILS_TEXT="Initial Git Checks for $TARGET_REPO

"
    for item in "${RESULTS[@]}"; do
        DETAILS_TEXT+="* $(add_emoji "$item")"$'\n'
    done

    local ESCAPED_DETAILS
    ESCAPED_DETAILS=$(escape_json "$DETAILS_TEXT")

    run_curl DELETE "$REPORT_URL" "" || true

    local PAYLOAD
    read -r -d '' PAYLOAD <<EOF || true
{
  "key":      "$REPORT_KEY",
  "title":    "* PR Bot: Git Checks ($BB_REPO)",
  "reporter": "jenkins",
  "result":   $( (( ALL_PASS == 1 )) && echo '"PASS"' || echo '"FAIL"' ),
  "details":  "${ESCAPED_DETAILS}",
  "data": [
    {
      "title": "Pre-Build Report",
      "type": "LINK",
      "value": {
        "linktext": "results.html",
        "href": "${RESULTS_HTML_URL}"
      }
    }
  ]
}
EOF

    run_curl PUT "$REPORT_URL" "$PAYLOAD"
    log "Git Checks report published (result=$( (( ALL_PASS == 1 )) && echo PASS || echo FAIL ))"
}

# ============================================================================
#  Build Checks (lint, docs, gen-buildfiles) - runs on SDK_ROOT
# ============================================================================

run_build_checks() {
    log "========== Running Build Checks on $SDK_ROOT =========="
    cd "$WORK_DIR/$SDK_ROOT"
    log "Working in: $(pwd)"

    IFS=, read -r -a devices <<< "$DEVICE_LIST"

    for device in "${devices[@]}"; do
        logdir="$LOG_DIR/${device}"
        mkdir -p "$logdir"
        for target in lint docs gen-buildfiles; do
            touch "${logdir}/${target}.log"
            touch "${logdir}/${target}_error.log"
        done
    done

    log "Installing npm packages..."
    npm config set proxy http://webproxy.ext.ti.com:80
    npm config set https-proxy http://webproxy.ext.ti.com:80
    npm config set registry https://registry.npmjs.org/
    npm config set strict-ssl false
    npm install

    local targets=(lint docs gen-buildfiles)

    for target in "${targets[@]}"; do
        local make_target=$target
        [[ "$target" == "gen-buildfiles" ]] && make_target="gen-buildfiles-all"

        log "Running '$make_target' checks for all devices..."

        for device in "${devices[@]}"; do
            log "  [${device}] Checking ${make_target}..."

            local log_stdout="$LOG_DIR/${device}/${target}.log"
            local log_stderr="$LOG_DIR/${device}/${target}_error.log"

            local diff_before=0
            git diff --quiet || diff_before=1

            make -s "$make_target" TI_SDK_DEVICE="$device" >>"$log_stdout" 2>>"$log_stderr" || true

            local diff_after=0
            git diff --quiet || diff_after=1

            if [[ $diff_before -ne $diff_after ]]; then
                echo "ERROR: you forgot to run make ${make_target}" >> "$log_stderr"
                echo "Changed files:" >> "$log_stderr"
                git diff --name-only >> "$log_stderr" 2>&1
            fi

            log "  [${device}] ${make_target} check complete"
        done
    done

    log "Build checks completed"
}

# ============================================================================
#  Publish SDK Checks Report
# ============================================================================

publish_sdk_checks_report() {
    log "Publishing SDK Checks report to Bitbucket..."

    cd "$WORK_DIR/$TARGET_REPO_PATH"
    local COMMIT_ID
    COMMIT_ID=$(git rev-parse HEAD)

    IFS=, read -r -a devices <<< "$DEVICE_LIST"
    local targets=(lint docs gen-buildfiles)

    local ANY_FAIL=0
    for device in "${devices[@]}"; do
        for t in "${targets[@]}"; do
            [[ -s "$LOG_DIR/$device/${t}_error.log" ]] && ANY_FAIL=1
        done
    done

    local STATUS
    STATUS=$( (( ANY_FAIL )) && echo "❌ FAIL" || echo "✅ PASS" )

    local BOARD_RESULTS=""
    for device in "${devices[@]}"; do
        BOARD_RESULTS+=$(printf "%-8s: " "${device}")
        local results=()
        for t in "${targets[@]}"; do
            if [[ -s "$LOG_DIR/$device/${t}_error.log" ]]; then
                results+=("${t}: ❌")
            else
                results+=("${t}: ✅")
            fi
        done
        IFS=' | '; BOARD_RESULTS+="${results[*]}"
        unset IFS
        BOARD_RESULTS+=$'\n'
    done

    local DETAILS_SDK="

 Overall Status: ${STATUS}

 Board Results:
${BOARD_RESULTS}
 SDK-specific Results:
For detailed analysis and logs, check the full HTML report below.

"

    local ESC_SDK
    ESC_SDK=$(escape_json "$DETAILS_SDK")

    local REPORT_URL_SDK="$BASE_INSIGHTS/projects/$BB_PROJECT/repos/$BB_REPO/commits/$COMMIT_ID/reports/sdk_checks"
    run_curl DELETE "$REPORT_URL_SDK" "" || true

    local PAYLOAD_SDK
    read -r -d '' PAYLOAD_SDK <<EOF || true
{
  "key": "sdk_checks",
  "title": "* SDK & Board Checks",
  "reporter": "jenkins",
  "result": $( (( ANY_FAIL == 0 )) && echo '"PASS"' || echo '"FAIL"' ),
  "details": "${ESC_SDK}",
  "data": [
    {
      "title": "Full HTML Report",
      "type": "LINK",
      "value": {
        "linktext": "Build Report",
        "href": "${RESULTS_HTML_URL}"
      }
    }
  ]
}
EOF

    run_curl PUT "$REPORT_URL_SDK" "$PAYLOAD_SDK"
    log "SDK Checks report published (result=$STATUS)"

    (( ANY_FAIL )) && ALL_PASS=0
}

# ============================================================================
#  Generate HTML Reports
# ============================================================================

generate_html_report() {
    log "Generating HTML report..."

    local HTML_PATH="$ARTIFACTS_DIR/results.html"
    IFS=, read -r -a devices <<< "$DEVICE_LIST"
    local targets=(lint docs gen-buildfiles)

    local ANY_FAIL=0
    for line in "${RESULTS[@]}"; do
        [[ $line == FAIL* ]] && ANY_FAIL=1
    done
    for device in "${devices[@]}"; do
        for t in "${targets[@]}"; do
            [[ -s "$LOG_DIR/$device/${t}_error.log" ]] && ANY_FAIL=1
        done
    done

    cat > "$HTML_PATH" <<'EOF'
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CI Report</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; padding: 20px; }
        .container { max-width: 1200px; margin: 0 auto; background: white; border-radius: 12px; box-shadow: 0 20px 40px rgba(0,0,0,0.1); overflow: hidden; }
        .header { padding: 30px; text-align: center; color: white; }
        .header.pass { background: linear-gradient(135deg, #11998e 0%, #38ef7d 100%); }
        .header.fail { background: linear-gradient(135deg, #ee0979 0%, #ff6a00 100%); }
        .header h1 { font-size: 2.5rem; font-weight: 700; margin-bottom: 10px; }
        .content { padding: 30px; }
        .section { margin-bottom: 30px; }
        .section h2 { color: #2d3748; font-size: 1.5rem; margin-bottom: 20px; padding-bottom: 10px; border-bottom: 2px solid #e2e8f0; }
        .summary-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 15px; margin-bottom: 20px; }
        .summary-item { padding: 15px 20px; border-radius: 8px; display: flex; align-items: center; font-weight: 500; }
        .summary-item .icon { margin-right: 10px; }
        .summary-item.pass { background: #f0fff4; border-left: 4px solid #38a169; color: #22543d; }
        .summary-item.fail { background: #fff5f5; border-left: 4px solid #e53e3e; color: #742a2a; }
        .summary-item.warn { background: #fffbeb; border-left: 4px solid #f59e0b; color: #92400e; }
        .summary-item.info { background: #eff6ff; border-left: 4px solid #3b82f6; color: #1e40af; }
        .devices-table { width: 100%; border-collapse: collapse; background: white; border-radius: 8px; overflow: hidden; box-shadow: 0 4px 6px rgba(0,0,0,0.05); }
        .devices-table th { background: #4a5568; color: white; padding: 15px; text-align: left; font-weight: 600; }
        .devices-table td { padding: 12px 15px; border-bottom: 1px solid #e2e8f0; }
        .status-badge { display: inline-flex; align-items: center; padding: 4px 8px; border-radius: 4px; font-size: 0.875rem; font-weight: 500; }
        .status-badge.pass { background: #c6f6d5; color: #22543d; }
        .status-badge.fail { background: #fed7d7; color: #742a2a; }
        .log-item { margin-bottom: 15px; border: 1px solid #e2e8f0; border-radius: 8px; overflow: hidden; }
        .log-header { background: #f7fafc; padding: 15px 20px; cursor: pointer; display: flex; justify-content: space-between; }
        .log-header.error { background: #fff5f5; border-left: 4px solid #e53e3e; }
        .log-content { display: none; max-height: 400px; overflow-y: auto; background: #1a202c; }
        .log-content.show { display: block; }
        .log-content pre { margin: 0; padding: 20px; color: #e2e8f0; font-family: monospace; font-size: 0.875rem; white-space: pre-wrap; }
    </style>
</head>
<body>
    <div class="container">
EOF

    cat >> "$HTML_PATH" <<EOF
        <div class="header $( (( ANY_FAIL )) && echo fail || echo pass )">
            <h1>$( (( ANY_FAIL )) && echo "❌ PRE-BUILD FAILED" || echo "✅ PRE-BUILD PASSED" )</h1>
            <div>PR #${PR_ID} ($TARGET_REPO) - CI Report</div>
        </div>
        <div class="content">
            <div class="section">
                <h2>Git Checks ($BB_REPO)</h2>
                <div class="summary-grid">
EOF

    for line in "${RESULTS[@]}"; do
        local css_class="info" emoji="" display_line="$line"
        if [[ $line == PASS:* ]]; then
            css_class="pass"; emoji="✅"; display_line="${line#PASS: }"
        elif [[ $line == FAIL:* ]]; then
            css_class="fail"; emoji="❌"; display_line="${line#FAIL: }"
        elif [[ $line == WARN:* ]]; then
            css_class="warn"; emoji="⚠️"; display_line="${line#WARN: }"
        elif [[ $line == INFO:* ]]; then
            css_class="info"; emoji="ℹ️"; display_line="${line#INFO: }"
        fi
        cat >> "$HTML_PATH" <<EOF
                    <div class="summary-item $css_class">
                        <span class="icon">$emoji</span>
                        <span>$display_line</span>
                    </div>
EOF
    done

    cat >> "$HTML_PATH" <<EOF
                </div>
            </div>
            <div class="section">
                <h2>Device-Specific Checks</h2>
                <table class="devices-table">
                    <thead>
                        <tr><th>Device</th><th>Lint</th><th>Docs</th><th>Gen-buildfiles</th></tr>
                    </thead>
                    <tbody>
EOF

    for device in "${devices[@]}"; do
        echo "                        <tr>" >> "$HTML_PATH"
        echo "                            <td><strong>$device</strong></td>" >> "$HTML_PATH"
        for t in "${targets[@]}"; do
            local errfile="$LOG_DIR/$device/${t}_error.log"
            if [[ -s "$errfile" ]]; then
                echo '                            <td><span class="status-badge fail">❌ FAIL</span></td>' >> "$HTML_PATH"
            else
                echo '                            <td><span class="status-badge pass">✅ PASS</span></td>' >> "$HTML_PATH"
            fi
        done
        echo "                        </tr>" >> "$HTML_PATH"
    done

    cat >> "$HTML_PATH" <<EOF
                    </tbody>
                </table>
            </div>
            <div class="section">
                <h2>Detailed Logs</h2>
EOF

    local log_count=0
    for device in "${devices[@]}"; do
        for t in "${targets[@]}"; do
            local logfile="$LOG_DIR/$device/${t}.log"
            local errfile="$LOG_DIR/$device/${t}_error.log"
            if [[ -s "$logfile" ]] || [[ -s "$errfile" ]]; then
                log_count=$((log_count + 1))
                local is_error="" log_content=""
                if [[ -s "$errfile" ]]; then
                    is_error="error"
                    log_content=$(sed -e 's/&/\&amp;/g' -e 's/</\&lt;/g' -e 's/>/\&gt;/g' < "$errfile")
                elif [[ -s "$logfile" ]]; then
                    log_content=$(sed -e 's/&/\&amp;/g' -e 's/</\&lt;/g' -e 's/>/\&gt;/g' < "$logfile")
                fi
                cat >> "$HTML_PATH" <<EOF
                <div class="log-item">
                    <div class="log-header $is_error" onclick="toggleLog('log-$log_count')">
                        <div>$device - $t $( [[ -n "$is_error" ]] && echo "(ERROR)" )</div>
                        <div id="toggle-$log_count">+</div>
                    </div>
                    <div class="log-content" id="log-$log_count">
                        <pre>$log_content</pre>
                    </div>
                </div>
EOF
            fi
        done
    done

    cat >> "$HTML_PATH" <<'EOF'
            </div>
        </div>
    </div>
    <script>
        function toggleLog(logId) {
            const el = document.getElementById(logId);
            el.classList.toggle('show');
        }
    </script>
</body>
</html>
EOF

    log "HTML report generated at $HTML_PATH"
}

# ============================================================================
#  Main Execution
# ============================================================================

main() {
    log "========== PRBot Starting Inside Container (Multi-Repo) =========="
    log "Target Repo: $TARGET_REPO | PR: $PR_ID | Devices: $DEVICE_LIST | AI: $AI_REPORT"
    log "Manifest: $MANIFEST_TEAM/$MANIFEST_FILE"
    log "Artifacts: $ARTIFACTS_DIR"

    # Create artifacts directory
    mkdir -p "$ARTIFACTS_DIR"

    # Step 1: Clone with repo tool (from common_utils.sh)
    if ! clone_with_repo_tool "$WORK_DIR" "$MANIFEST_URL" "$MANIFEST_TEAM" "$MANIFEST_FILE"; then
        log "ERROR: clone_with_repo_tool failed"
        exit 1
    fi

    # Step 2: Verify TARGET_REPO_PATH was set by clone_with_repo_tool
    if [[ -z "$TARGET_REPO_PATH" ]]; then
        log "ERROR: TARGET_REPO_PATH not set (repo '$TARGET_REPO' not found in manifest)"
        exit 1
    fi

    # Step 3: Checkout target PR using virtual ref (from common_utils.sh)
    if ! checkout_pr "$WORK_DIR/$TARGET_REPO_PATH" "$PR_ID" "target"; then
        log "ERROR: Failed to checkout target PR"
        exit 1
    fi

    # Step 4: Parse and checkout dependent PRs from description (from common_utils.sh)
    parse_and_checkout_dependent_prs "$WORK_DIR" "$PR_DESCRIPTION" "$TARGET_REPO"

    # Step 4.5: Generate and print repo-revs for debugging
    local REPO_REVS_FILE="${ARTIFACTS_DIR}/repo-revs.txt"
    generate_repo_revs "$WORK_DIR" "$REPO_REVS_FILE"
    log "=== Repository versions after all checkouts: ==="
    cat "$REPO_REVS_FILE"
    log "=== End of repository list ==="

    # Step 5: Run git checks (on target repo only)
    run_git_checks

    # Step 6: Publish git checks report
    publish_git_checks_report

    # Step 7: Run build checks (on SDK root)
    run_build_checks

    # Step 8: Generate HTML report
    generate_html_report

    # Step 9: Publish SDK checks report
    publish_sdk_checks_report

    log "========== PRBot Completed =========="
    log "Overall result: $( (( ALL_PASS == 1 )) && echo PASS || echo FAIL )"

    (( ALL_PASS == 1 )) && exit 0 || exit 1
}

main "$@"
