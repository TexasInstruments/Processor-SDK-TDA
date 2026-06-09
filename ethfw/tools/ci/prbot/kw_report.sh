#!/usr/bin/env bash
set -uo pipefail

#  Configuration 
: "${PULL_REQUEST_ID:?PULL_REQUEST_ID must be set}"
: "${COMMIT_ID:?COMMIT_ID must be set}"
: "${BB_TOKEN:?BB_TOKEN must be set}"

# Bitbucket & repo info
export PR_ID="$PULL_REQUEST_ID"
export BB_PROJECT="PROCESSOR-SDK"
export BB_REPO="mcu_sdk"
export BASE_API="https://bitbucket.itg.ti.com/rest/api/1.0"
export BASE_INSIGHTS="https://bitbucket.itg.ti.com/rest/insights/1.0"

# Jenkins workspace & clone settings
export JENKINS_GIT_DIR="/scratch/${JOB_NAME}-${BUILD_NUMBER}"
export SCRIPTS_BRANCH="${SCRIPTS_BRANCH:-main}"
export RESULTS_HTML="results.html"
export TOOLS_DIR="/scratch/mcusdk_tools/CCS"
export SCRIPT_NAME="kw_container.sh"
export SCRIPT_DIR="$JENKINS_GIT_DIR"
export DOCKER_TOOLS_DIR="/home/mcusdkdev/ti"
export DOCKER_WS="/home/mcusdkdev/ti/workarea"
export KW_JENKINS="/scratch/mcusdk_kw_new_install"
export KW_DOCKER="/home/mcusdkdev/ti/mcusdk_kw_new_install"

log(){ echo "[$(date +'%T')] $*"; }

run_curl() {
  local method=$1 url=$2 data=$3 tmp_body
  tmp_body=$(mktemp)
  local code
  if [[ "$method" == "DELETE" ]]; then
    code=$(curl -s -w "%{http_code}" -o "$tmp_body" \
      -X DELETE -H "Authorization: Bearer $BB_TOKEN" \
      "$url" || true)
  else
    code=$(curl -s -w "%{http_code}" -o "$tmp_body" \
      -X PUT \
      -H "Content-Type: application/json" \
      -H "Authorization: Bearer $BB_TOKEN" \
      -d "$data" \
      "$url")
  fi
  echo "HTTP $method → $url"
  echo "  Status: $code"
  echo "  Body:   $(<"$tmp_body")"
  rm "$tmp_body"
  (( code >= 400 )) && { echo "❌  $method failed ($code)"; exit 1; }
}


# clone the repo here 
log "Cloning $SCRIPTS_BRANCH@$COMMIT_ID into $JENKINS_GIT_DIR"
git clone --single-branch --branch "$tag_info" \
  "ssh://git@bitbucket.itg.ti.com/$BB_PROJECT/$BB_REPO.git" 

# move inside repo
cd "$BB_REPO"
git checkout "$COMMIT_ID"
git submodule update --init --recursive


# take the git diff and save the file name and path wrt root in a json in ../ 

#  Fetch origin/main so diff against it works
echo "[CI] Fetching origin/main..."
git fetch origin main:refs/remotes/origin/main

DIFF_FILES=()
#  List all Added (A) or Modified (M) files in this PR
echo "[CI] Listing A/M files vs origin/main..."
mapfile -t DIFF_FILES < <(
  git diff --diff-filter=AM --name-only origin/main...HEAD
)
echo "[CI] Raw diff list (${#DIFF_FILES[@]} files):"
for f in "${DIFF_FILES[@]}"; do
  echo "   • $f"
done

#  Parse kw_ignore.json via Python
IGNORE_FILE="../kw_ignore.json"
IGNORED=()

echo "[CI] Loading ignore list from $IGNORE_FILE..."
mapfile -t IGNORED < <(python3 - "$IGNORE_FILE" <<'PYCODE'
import sys, json
ignore_f = sys.argv[1]
with open(ignore_f) as f:
  for p in json.load(f):
      print(p.rstrip('/'))  # strip any trailing slash
PYCODE
)
echo "[CI] Ignore list contains (${#IGNORED[@]}) entries:"


if (( ${#IGNORED[@]} )); then
  for ig in "${IGNORED[@]}"; do
    echo "   ✘ $ig"
  done
else
  echo "[CI] No ignored entries."
fi


# Filter out ignored + non-.c/.h files
ADDED_FILES=()
echo "[CI] Filtering diff list for .c/.h and excluding ignored..."
for src in "${DIFF_FILES[@]}"; do
  # only .c or .h
  if [[ ! "$src" =~ \.(c|h)$ ]]; then
    echo "   - Skipping non-C file: $src"
    continue
  fi

  # check against ignore list (file or directory)
  if (( ${#IGNORED[@]} )); then
    skip=false
    for ig in "${IGNORED[@]}"; do
      if [[ "$src" == "$ig" || "$src" == "$ig/"* ]]; then
        echo "   - Ignoring $src (matched ignore entry: $ig)"
        skip=true
        break
      fi
    done
    $skip && continue
  fi
  echo "   + Including $src"
  ADDED_FILES+=("$src")
done

echo "[CI] Final list to process (${#ADDED_FILES[@]} files):"
for f in "${ADDED_FILES[@]}"; do
  echo "   ▶ $f"
done

# Emit JSON metadata
JSON_PATH="../file_path_kw.json"
echo "[CI] Writing metadata to $JSON_PATH..."
{
  printf '[\n'
  first=true
  for src in "${ADDED_FILES[@]}"; do
    [[ "$first" = true ]] && first=false || printf ',\n'
    name=$(basename "$src")
    esc=${src//\\/\\\\}       # escape backslashes
    esc=${esc//\"/\\\"}       # escape quotes
    printf '  { "file": "%s", "path": "%s" }' "$name" "$esc"
  done
  printf '\n]\n'
} > "$JSON_PATH"

echo "[CI] Collected ${#ADDED_FILES[@]} files into metadata into $JSON_PATH"

# after fetching files and saving data in file_path_kw.json more back and run kw inside container post mounting 
cd ..

export SCRIPT_DIR="$JENKINS_GIT_DIR"
tar -C "$HOME" -cf - .ssh | tar -C "$SCRIPT_DIR" -xf - || true
# first build the image 
export IMAGE_NAME="${JOB_NAME:-ci-image}"
export HOST_UID=$(id -u)
export HOST_GID=$(id -g)

echo "[CI] Building docker image $IMAGE_NAME..."
docker build -t "$IMAGE_NAME" \
  --build-arg HOST_UID="$HOST_UID" \
  --build-arg HOST_GID="$HOST_GID" \
  "$SCRIPT_DIR"
rm -rf "$SCRIPT_DIR/.ssh"


# mount the kw directory inside the container and the cloned directory inside the container 


docker run --rm \
  -e HOST_UID="$HOST_UID" \
  -e HOST_GID="$HOST_GID" \
  -v "$TOOLS_DIR:$DOCKER_TOOLS_DIR" \
  -v "$SCRIPT_DIR:$DOCKER_WS" \
  -v "$KW_JENKINS:$KW_DOCKER" \
  "$IMAGE_NAME" bash -c "\
    set -uo pipefail && \
    # all of this runs as mcusdkdev:
    sudo su - mcusdkdev -c '\
      cd $DOCKER_WS && \
      ./$SCRIPT_NAME \
    '\
"


RESULTS_HTML_URL="https://jenkins-proc.itg.ti.com/view/JACINTO_MCU_SDK/job/mcu_sdk_tda54_kw_build/${BUILD_NUMBER}/artifact/artifacts/results.html"
RESULTS_XML_URL="https://jenkins-proc.itg.ti.com/view/JACINTO_MCU_SDK/job/mcu_sdk_tda54_kw_build/${BUILD_NUMBER}/artifact/artifacts/report.xml"
FIL_RESULTS_XML_URL="https://jenkins-proc.itg.ti.com/view/JACINTO_MCU_SDK/job/mcu_sdk_tda54_kw_build/${BUILD_NUMBER}/artifact/artifacts/filtered_report.xml"
echo "${RESULTS_HTML_URL}"
# first put a klockwork report with the results.html link
# Send the report to BitBucket annotations
REPORT_KEY_KW="kw_checks"
REPORT_URL_KW="$BASE_INSIGHTS/projects/$BB_PROJECT/repos/$BB_REPO/commits/$COMMIT_ID/reports/$REPORT_KEY_KW"
run_curl DELETE "$REPORT_URL_KW" ""

# You have the file_path_kw.json and report.xml that's it to make html and even annotations both are here 
cp ./${BB_REPO}/build/tools/klocwork/report.xml .
cp ./${BB_REPO}/build/tools/klocwork/filtered_report.xml .

# convert .xml to .json 
export INPUT="report.xml"
export OUTPUT="report.json"
python3 - <<EOF
import xml.etree.ElementTree as ET
import json, os, sys


# Parse input
tree = ET.parse("$INPUT")
root = tree.getroot()


# Extract namespace URI (if any)
uri = root.tag[root.tag.find("{")+1:root.tag.find("}")] if "}" in root.tag else ""
ns = {"ns": uri} if uri else {}


problems = []
for pr in root.findall('ns:problem', ns):
    entry = {}
    for child in pr:
        tag = child.tag.split('}')[-1]  # strip namespace
        if tag == 'taxonomies':
            # collect all <taxonomy> elements as a list of dicts
            tax_list = []
            for tx in child.findall('ns:taxonomy', ns):
                # attributes: name, metaInf
                tax_list.append({
                    'name': tx.attrib.get('name', ''),
                    'metaInf': tx.attrib.get('metaInf', '')
                })
            entry['taxonomies'] = tax_list
        else:
            entry[tag] = child.text or ""
    # extra fields
    file_path = entry.get('file', '')
    entry['filename'] = os.path.basename(file_path)
    prefix = '/home/mcusdkdev/ti/workarea/$BB_REPO/'
    if file_path.startswith(prefix):
        entry['pathwrtroot'] = file_path[len(prefix):]
    else:
        entry['pathwrtroot'] = ""
    problems.append(entry)


# Wrap in an object and write out
out = {'errorList': {'problem': problems}}
with open("$OUTPUT", 'w') as f:
    json.dump(out, f, indent=2)


EOF

echo "✅ Converted '$INPUT' → '$OUTPUT'"

# Run the Python script and capture the output
SEVERITY_TABLE=$(python3 severity_analysis.py 2>/dev/null)

# Check if the Python script ran successfully
if [ $? -ne 0 ]; then
    SEVERITY_TABLE="Error: Unable to generate severity analysis table"
fi

# Properly escape the table for JSON (handle newlines, quotes, and backslashes)
SEVERITY_TABLE_JSON=$(echo "$SEVERITY_TABLE" | \
    sed 's/\\/\\\\/g' | \
    sed 's/"/\\"/g' | \
    sed ':a;N;$!ba;s/\n/\\n/g')

# Create the payload with the table in details section
read -r -d '' PAYLOAD_KW <<EOF
{
  "key":         "${REPORT_KEY_KW}",
  "title":       "• Klocwork report",
  "reporter":    "jenkins",
  "report_type": "SECURITY",
  "result":      "PASS",
  "details":     "${SEVERITY_TABLE_JSON}",
  "data": [
    {
      "title": "Klocwork Analysis report",
      "type":  "LINK",
      "value": {
        "linktext": "📊 results.html",
        "href":     "${RESULTS_HTML_URL}"
      }
    },
    {
      "title": "Full XML report",
      "type":  "LINK",
      "value": {
        "linktext": "📊 report.xml",
        "href":     "${RESULTS_XML_URL}"
      }
    },
    {
      "title": "Filtered XML report",
      "type":  "LINK",
      "value": {
        "linktext": "📊 filtered_report.xml",
        "href":     "${FIL_RESULTS_XML_URL}"
      }
    }
  ]
}
EOF

run_curl PUT "$REPORT_URL_KW" "$PAYLOAD_KW"
log "✅ Published Insights report: ${REPORT_KEY_KW})"


ANNOTATIONS_URL_KW="${REPORT_URL_KW}/annotations"
ANNOTATIONS_BODY=$(python3 - <<'PYCODE'
import json, os, sys
# Load the list of allowed filenames
try:
    allowed = json.load(open("file_path_kw.json"))
    allowed_files = { entry.get("file", "") for entry in allowed if entry.get("file") }
except Exception as e:
    print(f"❌ Failed to load 'file_path_kw.json': {e}", file=sys.stderr)
    sys.exit(1)
# Load report.json
try:
    report = json.load(open("report.json"))
    problems = report.get("errorList", {}).get("problem", [])
except Exception as e:
    print(f"❌ Failed to load or parse 'report.json': {e}", file=sys.stderr)
    sys.exit(1)

# Categorize problems by severity and sub-priority
high_anns = []      # severitylevel 1, 2, 5
medium_anns = []    # severitylevel 3, 6, 7, 8
low_anns = []       # all others

for p in problems:
    # Only annotate if this problem's filename is in file_path_kw.json
    fname = p.get("filename", "")
    if fname not in allowed_files:
        continue
    
    # Get severity level
    try:
        lvl = int(p.get("severitylevel", 0))
    except:
        lvl = 0
    
    # Build the annotation object
    msg    = p.get("message", "").replace('"', '\\"')
    code   = p.get("code", "").replace('"', '\\"')
    citing = p.get("citingStatus", "").replace('"', '\\"')
    
    ann = {
        "path":     p.get("pathwrtroot", ""),
        "line":     int(p.get("line", 0)),
        "message":  f'kw "{code}", "{citing}", "{msg}"',
        "severity_level": lvl  # Keep original level for sub-sorting
    }
    
    # Categorize by severity
    if lvl in (1, 2, 5):
        ann["severity"] = "HIGH"
        high_anns.append(ann)
    elif lvl in (3, 6, 7, 8):
        ann["severity"] = "MEDIUM"
        medium_anns.append(ann)
    else:
        ann["severity"] = "LOW"
        low_anns.append(ann)

# Define priority order for each category
high_priority = [1, 2, 5]       # HIGH: 1 first, then 2, then 5
medium_priority = [3, 6, 7, 8]  # MEDIUM: 3 first, then 6, then 7, then 8
low_priority = sorted(set(p.get("severitylevel", 0) for p in problems if p.get("severitylevel", 0) not in [1,2,3,5,6,7,8]))

# Sort function based on priority order
def sort_by_priority(anns, priority_order):
    def get_priority_index(ann):
        level = ann["severity_level"]
        try:
            return priority_order.index(level)
        except ValueError:
            return len(priority_order)  # Put unknown levels at the end
    return sorted(anns, key=get_priority_index)

# Sort each category by priority
high_anns = sort_by_priority(high_anns, high_priority)
medium_anns = sort_by_priority(medium_anns, medium_priority)
low_anns = sort_by_priority(low_anns, low_priority)

# Combine in order: HIGH -> MEDIUM -> LOW
final_anns = []
final_anns.extend(high_anns)
final_anns.extend(medium_anns)
final_anns.extend(low_anns)

# Limit to 400 annotations for Bitbucket
MAX_ANNOTATIONS = 400
if len(final_anns) > MAX_ANNOTATIONS:
    final_anns = final_anns[:MAX_ANNOTATIONS]
    print(f"⚠️  Limited to {MAX_ANNOTATIONS} annotations (had {len(high_anns) + len(medium_anns) + len(low_anns)} total)", file=sys.stderr)

# Clean up the annotations (remove the temporary severity_level field)
for ann in final_anns:
    del ann["severity_level"]

# Wrap the result
out = { "annotations": final_anns }

# Save to annotations.json
with open("annotations.json", "w") as f:
    json.dump(out, f, indent=2)

# Print to stdout
print(json.dumps(out, indent=2))
PYCODE
)


log "Publishing KW annotations to ${ANNOTATIONS_URL_KW}"
# allow script to continue even if annotations fail
set +e
HTTP_CODE=$(curl -s -w "%{http_code}" -X POST \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer ${BB_TOKEN}" \
  -d "${ANNOTATIONS_BODY}" \
  "${ANNOTATIONS_URL_KW}")
set -e

echo "  HTTP POST → ${ANNOTATIONS_URL_KW}"
echo "    Status: ${HTTP_CODE}"
if (( HTTP_CODE >= 400 )); then
  echo "⚠️ Annotations POST returned ${HTTP_CODE}, continuing"
else
  log "✅ Annotations published under ${REPORT_KEY_KW}."
fi


# server has some issues can't render html nicely therefore I ran it with a higher version of python in container
docker run --rm \
  -e HOST_UID="$HOST_UID" \
  -e HOST_GID="$HOST_GID" \
  -v "$TOOLS_DIR:$DOCKER_TOOLS_DIR" \
  -v "$SCRIPT_DIR:$DOCKER_WS" \
  -v "$KW_JENKINS:$KW_DOCKER" \
  "$IMAGE_NAME" bash -c "\
    set -uo pipefail && \
    # all of this runs as mcusdkdev:
    sudo su - mcusdkdev -c '\
      cd $DOCKER_WS && \
      sudo python kw_html.py \
    '\
"
#generate html here itself and done 



