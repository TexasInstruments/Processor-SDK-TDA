# ETHFW/ETHRTOS Promotion Script

This directory contains the automated promotion workflow for ETHFW/ETHRTOS projects.

## Overview

The promotion script automates the following workflow:

1. **Trigger Build Jobs** - Triggers multiple build jobs in parallel for different devices (j721e, j7200, j784s4)
2. **Wait for Builds** - Waits for all build jobs to complete successfully
3. **Trigger Test Jobs** - Triggers multiple test jobs in parallel (full regression tests)
4. **Parse Test Results** - Downloads and parses results.html from each test job
5. **Validate Results** - Validates that pass percentage is >= 95%
6. **Promote Branches** - If all validations pass, promotes multiple repositories from 'next' to 'main' branch

## Files

- **promotion_complete.py** - Complete promotion script (production-ready)
- **promotion.py** - Original/WIP version
- **ethfw_jenkins.cfg** - Jenkins server configuration and job definitions
- **ethfw_branch.cfg** - Repository branch promotion configuration
- **Dockerfile** - Docker container definition (if needed)

## Configuration Files

### ethfw_jenkins.cfg

Contains Jenkins server information and job configurations:

```ini
[JENKINS_SERVER_INFO]
jenkins_url = https://jenkins-epsw.itg.ti.com
jenkins_user = YOUR_USERNAME
jenkins_api_token = YOUR_API_TOKEN
test_jenkins_url = https://jenkins-proc.itg.ti.com
test_jenkins_user = YOUR_USERNAME
test_jenkins_api_token = YOUR_API_TOKEN
nas_base_url = http://epswnas.itg.ti.com/eth_rtos_cicd/release_area
build_job_name = ethrtos_ethfw_rtos
build_devices = j721e,j7200,j784s4
test_jobs = ethfw-j721e-pg1.1-full-test,ethfw-j7200-pg2.0-full-test,ethfw-j784s4-pg2.0-full-test
pass_percentage_threshold = 95.0
```

**IMPORTANT**: Update the credentials with your Jenkins username and API token!

### ethfw_branch.cfg

Defines which repositories to promote:

```ini
[ethfw]
clone_link = ssh://git@bitbucket.itg.ti.com/processor-sdk-vision/ethfw.git
next_branch = next
prod_branch = main

[enet-lld]
clone_link = ssh://git@bitbucket.itg.ti.com/processor-sdk-vision/enet-lld.git
next_branch = next
prod_branch = main
```

## Usage

### Basic Usage

```bash
# Run promotion with default configuration
python3 promotion_complete.py

# Run with specific branch
python3 promotion_complete.py --branch next

# Run with custom config files
python3 promotion_complete.py \
    --jenkins-config /path/to/ethfw_jenkins.cfg \
    --branch-config /path/to/ethfw_branch.cfg
```

### Dry Run Mode

Test the workflow without actually promoting branches:

```bash
python3 promotion_complete.py --dry-run
```

This will:
- Trigger builds and tests
- Validate test results
- Show what would be promoted
- But NOT execute git push commands

### Force Promote

Force promotion even if validation fails (use with caution):

```bash
python3 promotion_complete.py --force-promote
```

### Custom Artifacts Directory

Specify where to store logs and reports:

```bash
python3 promotion_complete.py --artifacts-dir /path/to/artifacts
```

## Command Line Arguments

| Argument | Description | Default |
|----------|-------------|---------|
| `--jenkins-config` | Path to Jenkins configuration file | `ethfw_jenkins.cfg` |
| `--branch-config` | Path to branch configuration file | `ethfw_branch.cfg` |
| `--branch` | Branch to build and test from | `next` |
| `--artifacts-dir` | Directory to store artifacts and reports | `artifacts` |
| `--dry-run` | Dry run mode - do not execute git promotions | False |
| `--force-promote` | Force promotion even if validation fails | False |

## Output

The script generates the following outputs:

### Console Output

Real-time logging to console showing:
- Build trigger status
- Build completion status
- Test trigger status
- Test completion status
- Test validation results
- Promotion decision
- Branch promotion status

### Log File

`artifacts/promotion.log` - Complete execution log

### Summary Report

`artifacts/promotion_summary.json` - JSON report containing:
```json
{
  "approved": true,
  "all_builds_passed": true,
  "all_tests_passed": true,
  "build_triggers": [...],
  "build_results": [...],
  "test_triggers": [...],
  "test_results": [...],
  "test_validations": [...],
  "promotions": [...]
}
```

## Workflow Details

### Phase 1: Trigger Build Jobs

Triggers `ethrtos_ethfw_rtos` job for each device with parameters:
- `product_family`: j721e, j7200, j784s4
- `release_build`: false (configurable)
- `BRANCH`: next (or specified branch)

Builds run in **parallel** for efficiency.

### Phase 2: Wait for Builds

Waits for all build jobs to complete with SUCCESS status.

**FAILURE HANDLING**: If any build fails, the promotion is immediately rejected and workflow stops.

### Phase 3: Trigger Test Jobs

Triggers test jobs in **parallel**:
- ethfw-j721e-pg1.1-full-test
- ethfw-j7200-pg2.0-full-test
- ethfw-j784s4-pg2.0-full-test

With parameters:
- `TEST_LABEL`: NIGHTLY
- `TEST_TYPE`: FULL
- `RELEASE_VERSION`: 0x_0x_0x_0x
- `INSTALLER_BUILD_ID`: latest

### Phase 4: Wait for Tests

Waits for all test jobs to complete.

### Phase 5: Validate Test Results

For each test job:
1. Downloads `artifacts/results.html`
2. Parses HTML to extract test summary
3. Validates pass percentage >= 95%
4. Records validation result

### Phase 6: Promotion Decision

Promotion is **APPROVED** if:
- All builds passed
- All tests passed
- All test pass percentages >= 95%

Or if `--force-promote` flag is used.

### Phase 7: Branch Promotions

If approved and not in dry-run mode:
1. Clone each repository
2. Checkout production branch (main)
3. Merge next branch into main (fast-forward if possible)
4. Push to remote

Promotes repositories:
- ethfw
- enet-lld
- enet-tsn-stack
- pdk

## Exit Codes

- `0` - Promotion approved and executed successfully
- `1` - Fatal error during execution
- `2` - Promotion rejected (builds or tests failed)

## Requirements

- Python 3.6+
- Git installed and configured
- SSH keys configured for Bitbucket access
- Jenkins API access credentials

## Troubleshooting

### Build Fails to Trigger

**Issue**: Job not triggered, no build number returned

**Solutions**:
- Check Jenkins credentials in config file
- Verify job name is correct
- Check Jenkins server is accessible
- Increase `queue_timeout_seconds` in config

### Test Results Not Found

**Issue**: Cannot fetch results.html artifact

**Solutions**:
- Verify test job completed successfully
- Check artifact path is correct
- Wait longer (artifact archiving may take time)
- Increase max_wait parameter in fetch_artifact

### Git Promotion Fails

**Issue**: Cannot push to repository

**Solutions**:
- Verify SSH keys are configured
- Check repository URLs in branch config
- Verify you have push permissions
- Check branch names are correct

### Pass Percentage Below Threshold

**Issue**: Tests pass but percentage < 95%

**Solutions**:
- Review failed test cases in results.html
- Fix failing tests and re-run
- Use `--force-promote` to bypass (not recommended)
- Lower threshold in config (not recommended)

## Best Practices

1. **Always run with --dry-run first** to validate workflow
2. **Review test results** before promotion
3. **Keep credentials secure** - use environment variables or secret management
4. **Monitor build and test jobs** in Jenkins UI
5. **Have rollback plan** ready in case of issues
6. **Test in non-production environment** first

## Example: Complete Workflow

```bash
# Step 1: Review configuration
cat ethfw_jenkins.cfg
cat ethfw_branch.cfg

# Step 2: Run in dry-run mode
python3 promotion_complete.py --branch next --dry-run

# Step 3: Review output
cat artifacts/promotion.log
cat artifacts/promotion_summary.json

# Step 4: If validation passes, run for real
python3 promotion_complete.py --branch next

# Step 5: Verify promotions
git ls-remote ssh://git@bitbucket.itg.ti.com/processor-sdk-vision/ethfw.git
```

## Jenkins Integration

To run this script from Jenkins:

```groovy
pipeline {
    agent any
    parameters {
        string(name: 'BRANCH', defaultValue: 'next', description: 'Branch to promote')
        booleanParam(name: 'DRY_RUN', defaultValue: true, description: 'Dry run mode')
    }
    stages {
        stage('Promotion') {
            steps {
                script {
                    sh """
                        python3 promotion_complete.py \
                            --branch ${params.BRANCH} \
                            ${params.DRY_RUN ? '--dry-run' : ''}
                    """
                }
            }
        }
        stage('Archive Results') {
            steps {
                archiveArtifacts artifacts: 'artifacts/**/*', allowEmptyArchive: false
            }
        }
    }
}
```

## Support

For issues or questions:
- Check this README
- Review logs in `artifacts/promotion.log`
- Check Jenkins job console output
- Contact the ETHFW/ETHRTOS team

## License

Internal use only - Texas Instruments
