#!/bin/bash
# =============================================================================
# EthFW PR Bot - Pre-build Git Checks and Reporting
# =============================================================================
# This script performs git checks and generates Bitbucket reports.
# The Jenkins job configuration will trigger downstream builds (ethfw_rtos, etc.)
# =============================================================================

set -e

# =============================================================================
# Configuration and Environment Variables
# =============================================================================
# Required environment variables:
# - BB_TOKEN: Bitbucket API token
# - PR_ID: Pull request ID
# - COMMIT_ID: Commit ID to test
# - BB_URL: Bitbucket base URL (e.g., https://bitbucket.itg.ti.com)
# - BB_PROJECT: Bitbucket project key (e.g., PROCESSOR-SDK-VISION)
# - BB_REPO: Bitbucket repo name (e.g., ethfw)
# - TARGET_BRANCH: Target branch for rebase check (default: master)
# - WORK_DIR: Working directory (default: current directory)

: "${BB_TOKEN:?BB_TOKEN not set}"
: "${PR_ID:?PR_ID not set}"
: "${COMMIT_ID:?COMMIT_ID not set}"
: "${BB_URL:?BB_URL not set}"
: "${BB_PROJECT:?BB_PROJECT not set}"
: "${BB_REPO:?BB_REPO not set}"
: "${TARGET_BRANCH:=master}"
: "${WORK_DIR:=$(pwd)}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ARTIFACTS_DIR="${ARTIFACTS_DIR:-${WORK_DIR}/artifacts}"
BASE_INSIGHTS="${BB_URL}/rest/insights/1.0"

mkdir -p "$ARTIFACTS_DIR"

# Array to store check results
declare -a RESULTS=()
ALL_PASS=1

# =============================================================================
# Helper Functions (from common_utils.sh)
# =============================================================================

# Timestamped logging
log() {
    echo "[$(date +'%T')] $*" >&2
}

# Escape string for JSON
escape_json() {
    printf '%s' "$1" | sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e ':a;N;$!ba;s/\n/\\n/g'
}

# HTTP request helper for Bitbucket API
run_curl() {
    local method=$1 url=$2 data=${3:-}
    local tmp_body code

    tmp_body=$(mktemp)

    case "$method" in
        DELETE)
            code=$(curl -s -w "%{http_code}" -o "$tmp_body" \
                -X DELETE -H "Authorization: Bearer $BB_TOKEN" \
                "$url" || true)
            ;;
        POST)
            code=$(curl -s -w "%{http_code}" -o "$tmp_body" \
                -X POST \
                -H "Content-Type: application/json" \
                -H "Authorization: Bearer $BB_TOKEN" \
                -d "$data" \
                "$url")
            ;;
        GET)
            code=$(curl -s -w "%{http_code}" -o "$tmp_body" \
                -H "Authorization: Bearer $BB_TOKEN" \
                "$url")
            cat "$tmp_body"
            ;;
        PUT)
            code=$(curl -s -w "%{http_code}" -o "$tmp_body" \
                -X PUT \
                -H "Content-Type: application/json" \
                -H "Authorization: Bearer $BB_TOKEN" \
                -d "$data" \
                "$url")
            ;;
    esac

    if [[ "$method" != "GET" ]]; then
        log "HTTP $method -> $url"
        log "  Status: $code"
        log "  Body:   $(<"$tmp_body")"
    fi

    rm -f "$tmp_body"
    (( code >= 400 )) && { log "ERROR: $method failed ($code)"; return 1; }
    return 0
}

# Add emoji prefix based on status
add_emoji() {
    local line="$1"
    case "$line" in
        PASS:*) echo "✅ ${line#PASS: }" ;;
        FAIL:*) echo "❌ ${line#FAIL: }" ;;
        WARN:*) echo "⚠️  ${line#WARN: }" ;;
        INFO:*) echo "ℹ️  ${line#INFO: }" ;;
        *) echo "$line" ;;
    esac
}

# =============================================================================
# Git Check Functions
# =============================================================================

check_rebase() {
    log "Checking if branch is rebased on $TARGET_BRANCH..."

    git fetch origin "$TARGET_BRANCH" 2>&1 || true

    local merge_base=$(git merge-base HEAD "origin/$TARGET_BRANCH" 2>/dev/null || echo "")
    local target_commit=$(git rev-parse "origin/$TARGET_BRANCH" 2>/dev/null || echo "")

    if [ -z "$merge_base" ] || [ -z "$target_commit" ]; then
        RESULTS+=("FAIL: Could not determine rebase status")
        ALL_PASS=0
        return 1
    fi

    if [ "$merge_base" != "$target_commit" ]; then
        RESULTS+=("FAIL: Branch is not rebased on $TARGET_BRANCH (please rebase)")
        ALL_PASS=0
        return 1
    else
        RESULTS+=("PASS: Branch is rebased on $TARGET_BRANCH")
        return 0
    fi
}

check_commit_count() {
    log "Checking commit count..."
    local max_commits="${1:-10}"

    git fetch origin "$TARGET_BRANCH" 2>&1 || true

    local commit_count=$(git rev-list --count "origin/$TARGET_BRANCH..HEAD" 2>/dev/null || echo "0")

    if [ "$commit_count" -gt "$max_commits" ]; then
        RESULTS+=("WARN: Branch has $commit_count commits (recommended max: $max_commits, consider squashing)")
        return 0
    else
        RESULTS+=("PASS: Branch has $commit_count commit(s) (within recommended range)")
        return 0
    fi
}

check_commit_message() {
    log "Checking commit message format..."

    local commit_msg=$(git log -1 --pretty=%B 2>/dev/null || echo "")

    if [ -z "$commit_msg" ]; then
        RESULTS+=("FAIL: Could not retrieve commit message")
        ALL_PASS=0
        return 1
    fi

    # Check minimum length
    if [ ${#commit_msg} -lt 10 ]; then
        RESULTS+=("FAIL: Commit message too short (minimum 10 characters)")
        ALL_PASS=0
        return 1
    fi

    # Check starts with alphanumeric
    if [[ ! "$commit_msg" =~ ^[A-Za-z0-9] ]]; then
        RESULTS+=("FAIL: Commit message should start with alphanumeric character")
        ALL_PASS=0
        return 1
    fi

    RESULTS+=("PASS: Commit message format is valid")
    return 0
}

check_fixes_line() {
    log "Checking for Fixes: line in commit message..."

    local commit_msg=$(git log -1 --pretty=%B 2>/dev/null || echo "")

    if echo "$commit_msg" | grep -qi "^Fixes:"; then
        RESULTS+=("PASS: Fixes: line found in commit message")
        return 0
    else
        RESULTS+=("WARN: No Fixes: line found (add 'Fixes: JIRA-XXX' if this fixes an issue)")
        return 0
    fi
}

# =============================================================================
# HTML Report Generation (matching inside_container.sh style)
# =============================================================================

generate_html_report() {
    log "Generating HTML report..."

    local HTML_PATH="$ARTIFACTS_DIR/results.html"

    cat > "$HTML_PATH" <<'EOF'
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>EthFW Pre-Build Report</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 12px;
            box-shadow: 0 20px 40px rgba(0,0,0,0.1);
            overflow: hidden;
        }
        .header { padding: 30px; text-align: center; color: white; }
        .header.pass { background: linear-gradient(135deg, #11998e 0%, #38ef7d 100%); }
        .header.fail { background: linear-gradient(135deg, #ee0979 0%, #ff6a00 100%); }
        .header h1 { font-size: 2.5rem; font-weight: 700; margin-bottom: 10px; }
        .header .subtitle { font-size: 1.1rem; opacity: 0.9; }
        .content { padding: 30px; }
        .section { margin-bottom: 30px; }
        .section h2 {
            color: #2d3748;
            font-size: 1.5rem;
            margin-bottom: 20px;
            padding-bottom: 10px;
            border-bottom: 2px solid #e2e8f0;
        }
        .summary-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 15px;
            margin-bottom: 20px;
        }
        .summary-item {
            padding: 15px 20px;
            border-radius: 8px;
            display: flex;
            align-items: center;
            font-weight: 500;
        }
        .summary-item .icon { margin-right: 10px; font-size: 1.2rem; }
        .summary-item.pass { background: #f0fff4; border-left: 4px solid #38a169; color: #22543d; }
        .summary-item.fail { background: #fff5f5; border-left: 4px solid #e53e3e; color: #742a2a; }
        .summary-item.warn { background: #fffbeb; border-left: 4px solid #f59e0b; color: #92400e; }
        .summary-item.info { background: #eff6ff; border-left: 4px solid #3b82f6; color: #1e40af; }
        .footer {
            margin-top: 30px;
            padding-top: 20px;
            border-top: 1px solid #e2e8f0;
            text-align: center;
            color: #718096;
            font-size: 0.875rem;
        }
    </style>
</head>
<body>
    <div class="container">
EOF

    cat >> "$HTML_PATH" <<EOF
        <div class="header $( (( ALL_PASS )) && echo pass || echo fail )">
            <h1>$( (( ALL_PASS )) && echo "✅ PRE-BUILD CHECKS PASSED" || echo "❌ PRE-BUILD CHECKS FAILED" )</h1>
            <div class="subtitle">PR #${PR_ID} (${BB_REPO}) - Git Checks Report</div>
        </div>
        <div class="content">
            <div class="section">
                <h2>Git Checks Summary</h2>
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
                <h2>Details</h2>
                <div class="summary-item info">
                    <span class="icon">📋</span>
                    <span><strong>Target Branch:</strong> $TARGET_BRANCH</span>
                </div>
                <div class="summary-item info">
                    <span class="icon">🔗</span>
                    <span><strong>Commit ID:</strong> $COMMIT_ID</span>
                </div>
                <div class="summary-item info">
                    <span class="icon">🔀</span>
                    <span><strong>Repository:</strong> $BB_PROJECT/$BB_REPO</span>
                </div>
            </div>
            <div class="footer">
                Generated by EthFW PR Bot • $(date +'%Y-%m-%d %H:%M:%S')
            </div>
        </div>
    </div>
</body>
</html>
EOF

    log "HTML report generated: $HTML_PATH"
}

# =============================================================================
# Bitbucket Report Publishing (matching inside_container.sh format)
# =============================================================================

publish_git_checks_report() {
    log "Publishing Git Checks report to Bitbucket..."

    local REPORT_KEY="git_checks"
    local REPORT_URL="$BASE_INSIGHTS/projects/$BB_PROJECT/repos/$BB_REPO/commits/$COMMIT_ID/reports/$REPORT_KEY"

    # Build details text
    local DETAILS_TEXT="Pre-Build Git Checks for $BB_REPO

"
    for item in "${RESULTS[@]}"; do
        DETAILS_TEXT+="* $(add_emoji "$item")"$'\n'
    done

    local ESCAPED_DETAILS
    ESCAPED_DETAILS=$(escape_json "$DETAILS_TEXT")

    # Delete existing report
    run_curl DELETE "$REPORT_URL" "" || true

    # Determine result status
    local RESULT_STATUS="PASS"
    (( ALL_PASS == 0 )) && RESULT_STATUS="FAIL"

    # Create payload
    local PAYLOAD
    read -r -d '' PAYLOAD <<EOF || true
{
  "key":      "$REPORT_KEY",
  "title":    "PR Bot: Git Checks ($BB_REPO)",
  "reporter": "jenkins",
  "result":   "$RESULT_STATUS",
  "details":  "${ESCAPED_DETAILS}",
  "data": []
}
EOF

    # Publish report
    if run_curl PUT "$REPORT_URL" "$PAYLOAD"; then
        log "Git Checks report published (result=$RESULT_STATUS)"
        return 0
    else
        log "ERROR: Failed to publish Git Checks report"
        return 1
    fi
}

# =============================================================================
# Main Execution Flow
# =============================================================================

main() {
    log "=========================================="
    log "EthFW PR Bot - Git Checks"
    log "=========================================="
    log "PR ID: ${PR_ID}"
    log "Commit ID: ${COMMIT_ID}"
    log "Repository: ${BB_PROJECT}/${BB_REPO}"
    log "Target Branch: ${TARGET_BRANCH}"
    log "Working Directory: ${WORK_DIR}"
    log "=========================================="

    # Change to work directory
    cd "$WORK_DIR"

    # Verify we're in a git repository
    if ! git rev-parse --git-dir > /dev/null 2>&1; then
        log "ERROR: Not in a git repository"
        exit 1
    fi

    log "Current directory: $(pwd)"
    log "Git status:"
    git status --short

    # =============================================================================
    # Run All Git Checks
    # =============================================================================
    log ""
    log "Running git checks..."
    log ""

    # Run each check (they update RESULTS array and ALL_PASS flag)
    check_rebase
    check_commit_count 10
    check_commit_message
    check_fixes_line

    log ""
    log "=========================================="
    log "Git Checks Summary:"
    log "=========================================="
    for result in "${RESULTS[@]}"; do
        log "  $result"
    done
    log "=========================================="
    log "Overall Status: $( (( ALL_PASS )) && echo PASS || echo FAIL )"
    log "=========================================="
    log ""

    # =============================================================================
    # Generate and Publish Reports
    # =============================================================================
    generate_html_report

    if publish_git_checks_report; then
        log "✓ Report published successfully to Bitbucket"
    else
        log "✗ Failed to publish report to Bitbucket"
    fi

    # =============================================================================
    # Final Exit Status
    # =============================================================================
    if (( ALL_PASS )); then
        log ""
        log "=========================================="
        log "✓ All pre-build checks PASSED"
        log "=========================================="
        exit 0
    else
        log ""
        log "=========================================="
        log "✗ Pre-build checks FAILED"
        log "=========================================="
        exit 1
    fi
}

# Run main function
main "$@"
