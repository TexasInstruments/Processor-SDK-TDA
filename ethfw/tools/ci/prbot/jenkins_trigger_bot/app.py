#!/usr/bin/env python3
"""
PR Bot - Jenkins Build Trigger for Bitbucket Webhooks
"""

import json
import logging
import os
import jenkins
import requests
from flask import Flask, request, jsonify
from dotenv import load_dotenv

# Logging setup
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Global variables
CONFIG = None
jenkins_client = None
bitbucket_client = None
llm_client = None

# Flask app
app = Flask(__name__)

def load_config(jobs_file="jenkins_job_configs.json"):
    """Load configuration from environment variables and jobs file"""
    # Load environment variables from .env file
    load_dotenv()

    # Build configuration from environment variables
    config = {
        "bitbucket": {
            "url": os.getenv("BITBUCKET_URL"),
            "username": os.getenv("BITBUCKET_USERNAME"),
            "password": os.getenv("BITBUCKET_PASSWORD")
        },
        "litellm": {
            "gateway_url": os.getenv("LITELLM_GATEWAY_URL"),
            "api_key": os.getenv("LITELLM_API_KEY"),
            "model": os.getenv("LITELLM_MODEL")
        },
        "jenkins": {
            "username": os.getenv("JENKINS_USERNAME"),
            "servers": {
                "proc": {
                    "url": os.getenv("JENKINS_PROC_URL"),
                    "password": os.getenv("JENKINS_PROC_PASSWORD")
                },
                "epsw": {
                    "url": os.getenv("JENKINS_EPSW_URL"),
                    "password": os.getenv("JENKINS_EPSW_PASSWORD")
                }
            }
        }
    }

    # Validate required configuration
    required_fields = [
        ("bitbucket.url", config["bitbucket"]["url"]),
        ("bitbucket.username", config["bitbucket"]["username"]),
        ("bitbucket.password", config["bitbucket"]["password"]),
        ("litellm.gateway_url", config["litellm"]["gateway_url"]),
        ("litellm.api_key", config["litellm"]["api_key"]),
        ("litellm.model", config["litellm"]["model"]),
        ("jenkins.username", config["jenkins"]["username"]),
        ("jenkins.proc.url", config["jenkins"]["servers"]["proc"]["url"]),
        ("jenkins.proc.password", config["jenkins"]["servers"]["proc"]["password"]),
        ("jenkins.epsw.url", config["jenkins"]["servers"]["epsw"]["url"]),
        ("jenkins.epsw.password", config["jenkins"]["servers"]["epsw"]["password"])
    ]

    missing_fields = [field for field, value in required_fields if not value]
    if missing_fields:
        raise ValueError(f"Missing required environment variables: {', '.join(missing_fields)}")

    # Load jobs configuration from JSON file
    try:
        with open(jobs_file, 'r') as f:
            config["jobs"] = json.load(f)
        logger.info(f"Loaded jobs configuration from {jobs_file}")
    except FileNotFoundError:
        logger.error(f"Jobs configuration file '{jobs_file}' not found")
        raise
    except json.JSONDecodeError as e:
        logger.error(f"Failed to parse jobs configuration file: {e}")
        raise

    return config

class JenkinsClient:
    """Jenkins client for triggering builds on multiple Jenkins servers"""

    def __init__(self, jenkins_config):
        self.username = jenkins_config['username']
        self.servers = jenkins_config['servers']
        logger.info(f"Initialized Jenkins client with {len(self.servers)} server(s)")

    def trigger_build(self, root_url, job_name, params, token):
        """Trigger a Jenkins build on the specified server"""
        try:
            # Determine which server credentials to use based on root_url
            password = None
            if 'jenkins-proc' in root_url:
                password = self.servers['proc']['password']
            elif 'jenkins-epsw' in root_url:
                password = self.servers['epsw']['password']
            else:
                logger.error(f"Unknown Jenkins server: {root_url}")
                return False

            # Create Jenkins connection for this specific job
            server = jenkins.Jenkins(root_url, username=self.username, password=password)
            server.build_job(job_name, parameters=params, token=token)
            logger.info(f"Triggered job '{job_name}' on {root_url} with params: {params}")
            return True
        except Exception as e:
            logger.error(f"Failed to trigger job '{job_name}' on {root_url}: {e}")
            return False

class BitbucketClient:
    """Bitbucket API client for fetching PR data and posting comments"""

    def __init__(self, bitbucket_config):
        self.base_url = bitbucket_config['url']
        self.auth = (bitbucket_config['username'], bitbucket_config['password'])
        logger.info(f"Initialized Bitbucket client for {self.base_url}")

    def get_pr_commits(self, project, repo, pr_number):
        """Get commits for a PR"""
        url = f"{self.base_url}/rest/api/1.0/projects/{project}/repos/{repo}/pull-requests/{pr_number}/commits"
        try:
            response = requests.get(url, auth=self.auth)
            response.raise_for_status()
            commits = response.json().get('values', [])
            return [{'message': c.get('message', ''), 'author': c.get('author', {}).get('name', '')} for c in commits]
        except Exception as e:
            logger.error(f"Failed to fetch PR commits: {e}")
            return []

    def get_pr_changes(self, project, repo, pr_number):
        """Get changed files in a PR"""
        url = f"{self.base_url}/rest/api/1.0/projects/{project}/repos/{repo}/pull-requests/{pr_number}/changes"
        try:
            response = requests.get(url, auth=self.auth)
            response.raise_for_status()
            changes = response.json().get('values', [])
            files = []
            for change in changes:
                path = change.get('path', {}).get('toString', '')
                change_type = change.get('type', '')
                if path:
                    files.append({'path': path, 'type': change_type})
            return files
        except Exception as e:
            logger.error(f"Failed to fetch PR changes: {e}")
            return []

    def get_comment_replies(self, project, repo, pr_number, parent_comment_id):
        """Get all replies to a specific comment"""
        url = f"{self.base_url}/rest/api/1.0/projects/{project}/repos/{repo}/pull-requests/{pr_number}/comments/{parent_comment_id}"
        try:
            response = requests.get(url, auth=self.auth)
            response.raise_for_status()
            comment_data = response.json()

            # Check if there are comments in the response (replies are in 'comments' field)
            comments = comment_data.get('comments', [])
            return comments
        except Exception as e:
            logger.error(f"Failed to fetch comment replies: {e}")
            return []

    def update_comment(self, project, repo, pr_number, comment_id, comment_text):
        """Update an existing comment"""
        url = f"{self.base_url}/rest/api/1.0/projects/{project}/repos/{repo}/pull-requests/{pr_number}/comments/{comment_id}"

        # First, get the current comment to get its version
        try:
            response = requests.get(url, auth=self.auth)
            response.raise_for_status()
            current_comment = response.json()
            version = current_comment.get('version')

            # Now update with the new text
            payload = {
                "text": comment_text,
                "version": version
            }

            response = requests.put(url, auth=self.auth, json=payload, headers={'Content-Type': 'application/json'})
            response.raise_for_status()
            logger.info(f"Updated comment {comment_id} on PR #{pr_number}")
            return True
        except Exception as e:
            logger.error(f"Failed to update comment: {e}")
            return False

    def post_comment(self, project, repo, pr_number, comment_text, parent_comment_id=None):
        """Post a comment to a PR, optionally as a reply to a parent comment"""
        url = f"{self.base_url}/rest/api/1.0/projects/{project}/repos/{repo}/pull-requests/{pr_number}/comments"
        payload = {"text": comment_text}

        # If parent_comment_id is provided, add it as a reply
        if parent_comment_id:
            payload["parent"] = {"id": parent_comment_id}

        try:
            response = requests.post(url, auth=self.auth, json=payload, headers={'Content-Type': 'application/json'})
            response.raise_for_status()
            if parent_comment_id:
                logger.info(f"Posted reply to comment {parent_comment_id} on PR #{pr_number}")
            else:
                logger.info(f"Posted comment to PR #{pr_number}")
            return True
        except Exception as e:
            logger.error(f"Failed to post comment to PR: {e}")
            return False

class LLMClient:
    """LiteLLM client for generating PR descriptions using LLM"""

    def __init__(self, llm_config):
        base_url = llm_config['gateway_url']
        # Ensure the URL ends with /chat/completions
        if not base_url.endswith('/chat/completions'):
            self.gateway_url = f"{base_url.rstrip('/')}/chat/completions"
        else:
            self.gateway_url = base_url
        self.api_key = llm_config['api_key']
        self.model = llm_config['model']
        logger.info(f"Initialized LLM client with model {self.model} at {self.gateway_url}")

    def generate_pr_description(self, commits, changed_files):
        """Generate PR description using LLM"""
        try:
            # Prepare commit information
            commit_msgs = [c['message'] for c in commits[:20]]  # Limit to 20 commits
            commit_summary = '\n'.join([f"- {msg}" for msg in commit_msgs])

            # Prepare file changes information
            file_summary = '\n'.join([f"- {f['path']} ({f['type']})" for f in changed_files[:30]])  # Limit to 30 files

            # Build prompt for LLM
            prompt = f"""You are a senior embedded software engineer reviewing a pull request for an MCU SDK project.
Based on the commits and file changes below, write a professional, technical PR description.

**Commits ({len(commits)} total):**
{commit_summary}

**Files Changed ({len(changed_files)} total):**
{file_summary}

Write a professional PR description in markdown format following this structure:

## Summary
Brief technical overview (2-3 sentences) explaining the purpose and scope of changes.

## Changes
Detailed bullet points covering:
- New features or implementations
- Bug fixes and corrections
- Performance improvements
- API changes or additions
- Testing updates

## Technical Details
Key technical points including:
- Architecture or design decisions
- Hardware-specific considerations
- Performance characteristics
- Compatibility notes

Requirements:
- Write in a professional, technical tone
- Make it crisp and to the point so that reviewer does not lose interest in reading it.
- NO emojis - keep it strictly professional
- Use precise technical terminology (registers, peripherals, DMA, interrupts, etc.)
- Use **bold** for component names and key terms
- Focus on embedded systems context (MCU SDK, drivers, HAL, BSP, etc.)"""


            # Call LiteLLM gateway
            headers = {
                'Authorization': f'Bearer {self.api_key}',
                'Content-Type': 'application/json'
            }

            payload = {
                'model': self.model,
                'messages': [
                    {'role': 'user', 'content': prompt}
                ],
                'temperature': 0.7,
                'max_tokens': 500
            }

            response = requests.post(self.gateway_url, headers=headers, json=payload, timeout=30)
            response.raise_for_status()

            result = response.json()
            description = result['choices'][0]['message']['content']

            # Add professional header with stats
            stats_line = f"**Commits:** {len(commits)} | **Files Changed:** {len(changed_files)}"

            full_description = f"""

{description}

"""

            logger.info("Successfully generated PR description using LLM")
            return full_description

        except Exception as e:
            logger.error(f"Failed to generate PR description with LLM: {e}")
            # Fallback to simple description
            return self._generate_fallback_description(commits, changed_files)

    def _generate_fallback_description(self, commits, changed_files):
        """Generate a simple fallback description if LLM fails"""
        description_parts = []

        stats_line = f"**Commits:** {len(commits)} | **Files Changed:** {len(changed_files)}"

        description_parts.append("## Pull Request Summary")
        description_parts.append("")
        description_parts.append(stats_line)
        description_parts.append("")
        description_parts.append("---")
        description_parts.append("")

        if commits:
            description_parts.append("### Recent Commits")
            description_parts.append("")
            for i, commit in enumerate(commits[:5], 1):
                msg = commit['message'].split('\n')[0]
                description_parts.append(f"- {msg}")
            description_parts.append("")

        description_parts.append("---")
        description_parts.append("*Note: Automated analysis unavailable - showing basic commit summary*")

        return "\n".join(description_parts)

def substitute_params(param_template, webhook_data):
    """Substitute placeholders in parameters with actual values"""
    result = {}

    # Handle params_template if provided (complex format with %tag% placeholders)
    if 'params_template' in param_template and 'default_tag' in param_template:
        # Parse the JSON template string
        template = json.loads(param_template['params_template'])
        default_tags = param_template['default_tag']

        # Override with branch name for the repo being built
        substitutions = default_tags.copy()
        substitutions[webhook_data['repo']] = webhook_data['branch']

        # Substitute %tag% placeholders
        for key, value in template.items():
            if isinstance(value, str):
                substituted_value = value
                for tag_key, tag_value in substitutions.items():
                    placeholder = f"%{tag_key}%"
                    substituted_value = substituted_value.replace(placeholder, str(tag_value))
                result[key] = substituted_value
            else:
                result[key] = value

    # Handle simple params format (with {placeholder} format)
    # Skip 'params_template' and 'default_tag' keys as they're metadata
    for key, value in param_template.items():
        if key in ['params_template', 'default_tag']:
            continue

        if isinstance(value, str):
            result[key] = value.format(
                branch=webhook_data['branch'],
                commit_id=webhook_data['commit_id'],
                pr_number=webhook_data['pr_number'],
                pr_description=webhook_data['pr_description'],
                project=webhook_data['project'],
                repo=webhook_data['repo'],
                target_repo=f"{webhook_data['project']}/{webhook_data['repo']}"
            )
        else:
            result[key] = value

    return result

def extract_webhook_data(payload):
    """Extract relevant data from Bitbucket webhook payload"""
    try:
        # Event type
        event_type = payload.get("eventKey", "")

        # PR data
        pr_data = payload.get("pullRequest", {})
        pr_number = pr_data.get("id")
        pr_title = pr_data.get("title", "")
        pr_description = pr_data.get("description", "")

        # Debug logging: Log the entire PR data structure to understand draft field location
        logger.info(f"PR Data Keys: {list(pr_data.keys())}")
        logger.info(f"PR draft field: {pr_data.get('draft')}")
        if 'attributes' in pr_data:
            logger.info(f"PR attributes: {pr_data.get('attributes')}")
        if 'properties' in pr_data:
            logger.info(f"PR properties: {pr_data.get('properties')}")
        logger.info(f"PR state: {pr_data.get('state')}")

        # Check if PR is in draft state
        # Bitbucket may use different fields depending on version:
        # - "draft" boolean field (newer versions)
        # - properties object with "mergeResult" containing "current": false
        # - attributes object with "draft" or "isDraft"
        # - state field (though typically just OPEN/MERGED/DECLINED)
        is_draft = False
        if pr_data.get("draft") is True:
            is_draft = True
            logger.info("Draft detected via 'draft' field")
        elif pr_data.get("attributes", {}).get("draft") is True:
            is_draft = True
            logger.info("Draft detected via 'attributes.draft' field")
        elif pr_data.get("attributes", {}).get("isDraft") is True:
            is_draft = True
            logger.info("Draft detected via 'attributes.isDraft' field")
        elif pr_data.get("properties", {}).get("draft") is True:
            is_draft = True
            logger.info("Draft detected via 'properties.draft' field")

        if not is_draft:
            logger.info("No draft status detected - PR will trigger builds if no other skip flags present")

        # Source branch information (from the PR author's branch/fork)
        from_ref = pr_data.get("fromRef", {})
        branch_name = from_ref.get("displayId", "")
        commit_id = from_ref.get("latestCommit", "")

        # Target repository information (where the PR is being merged into)
        # We use toRef for project/repo to correctly handle fork-based PRs
        # where fromRef points to the fork, not the main repository
        to_ref = pr_data.get("toRef", {})
        repository = to_ref.get("repository", {})
        repo_name = repository.get("slug")
        project_key = repository.get("project", {}).get("key")

        # Comment data (for comment events)
        comment_text = None
        comment_id = None
        if event_type in ["pr:comment:added", "pr:comment:edited"]:
            comment = payload.get("comment", {})
            comment_text = comment.get("text", "").strip()
            comment_id = comment.get("id")

        webhook_data = {
            "event_type": event_type,
            "project": project_key,
            "repo": repo_name,
            "branch": branch_name,
            "commit_id": commit_id,
            "pr_number": pr_number,
            "pr_title": pr_title,
            "pr_description": pr_description,
            "comment_text": comment_text,
            "comment_id": comment_id,
            "is_draft": is_draft
        }

        logger.info(f"Extracted webhook data: {webhook_data}")
        return webhook_data

    except Exception as e:
        logger.error(f"Error extracting webhook data: {e}")
        return None

@app.route('/webhook', methods=['POST'])
def webhook():
    """Handle Bitbucket webhook events"""
    payload = request.json

    if not payload:
        logger.warning("Received empty payload")
        return jsonify({"status": "error", "message": "Empty payload"}), 400

    # Extract webhook data
    webhook_data = extract_webhook_data(payload)

    if not webhook_data:
        return jsonify({"status": "error", "message": "Failed to extract webhook data"}), 400

    # Log extracted data
    logger.info(f"Event: {webhook_data['event_type']}")
    logger.info(f"Project: {webhook_data['project']}, Repo: {webhook_data['repo']}")
    logger.info(f"PR #{webhook_data['pr_number']}: {webhook_data['pr_title']}")
    logger.info(f"Branch: {webhook_data['branch']}, Commit: {webhook_data['commit_id']}")

    # Track if Build command was explicitly requested (to override [DNT])
    explicit_build_requested = False

    # Handle comment events - check for commands
    if webhook_data['event_type'] in ["pr:comment:added", "pr:comment:edited"]:
        comment_text = webhook_data.get('comment_text', '')

        # Check for "/describe" command to generate PR description using LLM
        if comment_text.lower() == "/describe":
            logger.info(f"Describe command detected in comment: '{comment_text}'")

            # Fetch PR data from Bitbucket
            project = webhook_data['project']
            repo = webhook_data['repo']
            pr_number = webhook_data['pr_number']
            comment_id = webhook_data.get('comment_id')

            commits = bitbucket_client.get_pr_commits(project, repo, pr_number)
            changed_files = bitbucket_client.get_pr_changes(project, repo, pr_number)

            if commits or changed_files:
                # Generate description using LLM
                suggested_description = llm_client.generate_pr_description(commits, changed_files)

                # Check if this is an edited comment and if we already replied
                existing_replies = bitbucket_client.get_comment_replies(project, repo, pr_number, comment_id)

                # Filter replies to find our bot's reply (from our bot user)
                bot_username = CONFIG['bitbucket']['username']
                bot_reply = None
                for reply in existing_replies:
                    if reply.get('author', {}).get('name') == bot_username:
                        bot_reply = reply
                        break

                if bot_reply and webhook_data['event_type'] == "pr:comment:edited":
                    # Update existing reply
                    reply_id = bot_reply.get('id')
                    bitbucket_client.update_comment(project, repo, pr_number, reply_id, suggested_description)
                    message = "AI-generated PR description updated"
                else:
                    # Post new reply to the "Describe" comment
                    bitbucket_client.post_comment(project, repo, pr_number, suggested_description, parent_comment_id=comment_id)
                    message = "AI-generated PR description posted as reply"

                return jsonify({
                    "status": "success",
                    "message": message
                }), 200
            else:
                return jsonify({
                    "status": "error",
                    "message": "Failed to fetch PR data"
                }), 500

        # Check for "/build" command
        elif comment_text.lower() == "/build":
            logger.info(f"Build command detected in comment: '{comment_text}'")
            explicit_build_requested = True
            # Continue to trigger build (will override [DNT] if present)
        else:
            logger.info(f"Ignoring comment that doesn't match known commands: '{comment_text}'")
            return jsonify({"status": "ignored", "message": "Comment does not contain known command"}), 200

    # Only trigger builds on PR open, update, or Build comment events
    elif webhook_data['event_type'] not in ["pr:opened", "pr:from_ref_updated"]:
        logger.info(f"Ignoring event type: {webhook_data['event_type']}")
        return jsonify({"status": "ignored", "message": "Event type not handled"}), 200

    # Check if PR should skip builds due to:
    # 1. Draft status (Bitbucket native draft PR)
    # 2. [DNT] - Do Not Test flag in title
    # 3. [WIP] - Work In Progress flag in title
    # This check is done AFTER handling "Describe" command, so these flags only block builds
    # However, explicit "Build" comment overrides all flags
    pr_title = webhook_data.get('pr_title', '')
    is_draft = webhook_data.get('is_draft', True)

    # Remove all spaces and convert to uppercase for checking
    pr_title_normalized = pr_title.replace(' ', '').upper()
    # Check for [DNT], (DNT), [WIP], or (WIP) in normalized string (handles any spacing)
    has_dnt = '[DNT]' in pr_title_normalized or '(DNT)' in pr_title_normalized
    has_wip = '[WIP]' in pr_title_normalized or '(WIP)' in pr_title_normalized

    # Determine if build should be skipped
    should_skip_build = (is_draft or has_dnt or has_wip) and not explicit_build_requested

    if should_skip_build:
        # Determine the reason for skipping
        if is_draft:
            skip_reason = "Draft PR"
        elif has_dnt:
            skip_reason = "[DNT] - Do Not Test"
        else:
            skip_reason = "[WIP] - Work In Progress"

        logger.info(f"Skipping build - {skip_reason}: '{pr_title}'")
        return jsonify({"status": "ignored", "message": f"PR marked as {skip_reason}"}), 200

    elif explicit_build_requested and (is_draft or has_dnt or has_wip):
        # Determine which flag was overridden
        if is_draft:
            override_flag = "Draft status"
        elif has_dnt:
            override_flag = "[DNT]"
        else:
            override_flag = "[WIP]"

        logger.info(f"Build command overrides {override_flag} - proceeding with build for PR: '{pr_title}'")

    # Look up job configuration using project/repo key
    job_key = f"{webhook_data['project']}/{webhook_data['repo']}"
    job_configs = CONFIG['jobs'].get(job_key)
    print("=========")
    print(job_key)
    print("=========")
    print(CONFIG['jobs'])
    if not job_configs:
        logger.warning(f"No job configuration found for {job_key}")
        return jsonify({"status": "error", "message": f"No configuration for {job_key}"}), 404

    # Trigger all enabled jobs
    triggered_jobs = []
    failed_jobs = []

    for job_config in job_configs:
        # Skip disabled jobs
        if job_config.get('disabled', False):
            logger.info(f"Skipping disabled job: {job_config['job_name']}")
            continue

        # Get job details
        root_url = job_config['root_url']
        job_name = job_config['job_name']
        token = job_config.get('token', '')

        # Merge params and other fields for substitution
        param_template = job_config.get('params', {}).copy()
        if 'params_template' in job_config:
            param_template['params_template'] = job_config['params_template']
        if 'default_tag' in job_config:
            param_template['default_tag'] = job_config['default_tag']

        # Substitute parameters
        build_params = substitute_params(param_template, webhook_data)

        # Trigger Jenkins build
        success = jenkins_client.trigger_build(root_url, job_name, build_params, token)

        if success:
            triggered_jobs.append(job_name)
            logger.info(f"Successfully triggered job: {job_name}")
        else:
            failed_jobs.append(job_name)
            logger.error(f"Failed to trigger job: {job_name}")

    # Return response based on results
    if triggered_jobs and not failed_jobs:
        return jsonify({
            "status": "success",
            "message": f"Triggered {len(triggered_jobs)} job(s)",
            "jobs": triggered_jobs
        }), 200
    elif triggered_jobs and failed_jobs:
        return jsonify({
            "status": "partial_success",
            "message": f"Triggered {len(triggered_jobs)} job(s), {len(failed_jobs)} failed",
            "triggered": triggered_jobs,
            "failed": failed_jobs
        }), 207
    else:
        return jsonify({
            "status": "error",
            "message": "Failed to trigger any builds",
            "failed": failed_jobs
        }), 500

if __name__ == '__main__':
    # Load configuration
    CONFIG = load_config()
    print(CONFIG)
    logger.info(f"Loaded configuration for {len(CONFIG['jobs'])} job(s)")

    # Initialize Jenkins client
    jenkins_client = JenkinsClient(CONFIG['jenkins'])

    # Initialize Bitbucket client
    bitbucket_client = BitbucketClient(CONFIG['bitbucket'])

    # Initialize LLM client
    llm_client = LLMClient(CONFIG['litellm'])

    # Start Flask app
    logger.info("Starting PR Bot on port 8000")
    app.run(host='0.0.0.0', port=8000, debug=False)
