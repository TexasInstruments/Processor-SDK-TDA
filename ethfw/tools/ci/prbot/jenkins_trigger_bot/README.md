# PR Bot

Flask webhook server that triggers Jenkins builds on Bitbucket PR events.

## How It Works

1. Bitbucket sends webhook POST to `/webhook` endpoint on:
   - `pr:opened` - When PR is created
   - `pr:from_ref_updated` - When PR is updated
   - `pr:comment:added` - When comment is posted:
     - `"Build"` - Triggers Jenkins build (case-insensitive)
     - `"Describe"` - Generates AI-powered PR description (case-insensitive)
2. Bot extracts: project, repo, branch, commit ID, PR number, PR title, PR description
3. **Skips build if PR title contains `[DNT]` (Do Not Test) or `[WIP] (Work In Progress)`** or **DRAFT** PRs - case-insensitive
4. Looks up job config using `project/repo` key from `config.json`
5. Substitutes parameters and triggers Jenkins build

## AI-Powered PR Description

When someone comments "Describe" on a PR, the bot will:

1. Fetch all commits and changed files from Bitbucket
2. Send the data to your configured LiteLLM gateway
3. Use an LLM (GPT-4, Claude, etc.) to generate an intelligent PR description
4. Post the description as a comment on the PR

The LLM analyzes:

- Commit messages and patterns
- Changed files and their types
- Technical context (MCU SDK, drivers, HAL, peripherals)
- Overall purpose and impact of the changes

If the LLM call fails, it falls back to a simple summary.

## Configuration

Edit `config.json`:

```json
{
  "bitbucket": {
    "url": "https://bitbucket.itg.ti.com",
    "username": "your_bitbucket_username",
    "password": "your_bitbucket_password_or_token"
  },
  "litellm": {
    "gateway_url": "https://your-litellm-gateway.com",
    "api_key": "your_api_key_here",
    "model": "gpt-4"
  },
  "jenkins": {
    "url": "https://jenkins-proc.itg.ti.com",
    "username": "your_username",
    "password": "your_api_token"
  },
  "jobs": {
    "PROJECT-KEY/repo_name": {
      "job_name": "jenkins_job_name",
      "token": "JOB_TOKEN",
      "params": {
        "PARAM_NAME": "{branch}",
        "COMMIT_ID": "{commit_id}",
        "PULL_REQUEST_ID": "{pr_number}",
        "PR_DESCRIPTION": "{pr_description}"
      }
    }
  }
}
```

## Run

```bash
python app.py
```

Server runs on `http://0.0.0.0:8000`

## Add New Jobs

Add entry to `config.json` under `jobs` with key `"PROJECT-KEY/repo_name"`.
Parameters support placeholders:
`{branch}`, `{commit_id}`, `{pr_number}`, `{pr_description}`
