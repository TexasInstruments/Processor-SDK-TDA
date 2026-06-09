#!/usr/bin/env python3
"""
ETHFW/ETHRTOS Promotion Script

This script automates the promotion workflow for ETHFW/ETHRTOS:
1. Trigger multiple build jobs in parallel for different devices
2. Wait for all builds to complete successfully
3. Trigger multiple test jobs in parallel
4. Parse results.html from each test job
5. Validate pass percentage (>= 90%)
6. Promote multiple repositories from 'next' to 'main' branch if all validations pass

Features:
- Parallel build and test execution
- NAS artifact management
- Comprehensive error handling
- Detailed logging and reporting
- Configuration-driven design
"""

import argparse
import base64
import configparser
import fcntl
import json
import logging
import os
import re
import shutil
import subprocess
import sys
import time
import urllib.request
import urllib.parse
from html.parser import HTMLParser
from typing import Any, Dict, List, Optional, Tuple

logging.basicConfig(level=logging.INFO, format='%(asctime)s %(levelname)s: %(message)s')
logger = logging.getLogger('ethfw_promotion')

DEFAULT_TIMEOUT = 60 * 60 * 6  # 6 hours


class ScriptLock:
    """Ensure only one instance of the promotion script runs at a time"""
    def __init__(self, lockfile: str):
        self.lockfile = lockfile
        self.lockfd = None

    def __enter__(self):
        self.lockfd = open(self.lockfile, 'w')
        try:
            fcntl.flock(self.lockfd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
            self.lockfd.write(f'{os.getpid()}\n')
            self.lockfd.flush()
            logger.info('Acquired script lock: %s', self.lockfile)
            return self
        except IOError:
            logger.error('Another instance of the promotion script is already running')
            logger.error('Lock file: %s', self.lockfile)
            raise RuntimeError('Another instance is already running')

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.lockfd:
            fcntl.flock(self.lockfd.fileno(), fcntl.LOCK_UN)
            self.lockfd.close()
            try:
                os.remove(self.lockfile)
                logger.info('Released script lock: %s', self.lockfile)
            except Exception:
                pass
        return False


def load_ini(path: str) -> configparser.ConfigParser:
    """Load INI configuration file"""
    p = configparser.ConfigParser()
    p.read(path)
    return p


def jenkins_auth_header(user: str, token: str) -> Dict[str, str]:
    """Create Jenkins authentication header"""
    auth = (user + ':' + token).encode('utf-8')
    return {'Authorization': 'Basic ' + base64.b64encode(auth).decode('ascii')}


def http_get(url: str, headers: Optional[Dict[str, str]] = None, timeout: int = 30) -> Tuple[int, bytes]:
    """Perform HTTP GET request"""
    req = urllib.request.Request(url)
    if headers:
        for k, v in headers.items():
            req.add_header(k, v)
    try:
        with urllib.request.urlopen(req, timeout=timeout) as rsp:
            return rsp.getcode(), rsp.read()
    except Exception as e:
        logger.debug('HTTP GET failed %s : %s', url, e)
        return 0, b''


def trigger_jenkins_job(base_url: str, job_name: str, params: Dict[str, str],
                       user: str, token: str, queue_timeout: int = 1800) -> Dict[str, Any]:
    """
    Trigger a Jenkins job and return build number

    Args:
        base_url: Jenkins base URL
        job_name: Name of the job to trigger
        params: Dictionary of job parameters
        user: Jenkins username
        token: Jenkins API token
        queue_timeout: Maximum seconds to wait for build number (default 1800)

    Returns:
        Dictionary with 'job', 'build', and 'queued' keys
    """
    job_api = base_url.rstrip('/') + f'/job/{job_name}'

    # Get last build number before triggering
    code, body = http_get(job_api + '/api/json?tree=lastBuild[number]', jenkins_auth_header(user, token))
    prev = None
    if code == 200:
        try:
            js = json.loads(body.decode('utf-8', 'ignore'))
            if 'lastBuild' in js and js['lastBuild']:
                prev = int(js['lastBuild'].get('number'))
        except Exception:
            prev = None

    # Build trigger URL with parameters
    qs = '&'.join(f'{urllib.parse.quote_plus(k)}={urllib.parse.quote_plus(v)}' for k, v in params.items())
    trigger = job_api + '/buildWithParameters' + (('?' + qs) if qs else '')

    logger.info('Triggering job %s', job_name)
    logger.debug('Trigger URL: %s', trigger)

    # Trigger the job
    try:
        req = urllib.request.Request(trigger, method='POST')
        hdrs = jenkins_auth_header(user, token)
        for k, v in hdrs.items():
            req.add_header(k, v)
        with urllib.request.urlopen(req, timeout=30) as resp:
            _ = resp.read()
    except Exception as e:
        logger.warning('Trigger request for %s failed: %s', job_name, e)

    # Poll for new build number
    deadline = time.time() + queue_timeout
    new_build = None
    while time.time() < deadline:
        code, body = http_get(job_api + '/api/json?tree=lastBuild[number]', jenkins_auth_header(user, token))
        if code == 200:
            try:
                js = json.loads(body.decode('utf-8', 'ignore'))
                if 'lastBuild' in js and js['lastBuild'] and js['lastBuild'].get('number') is not None:
                    num = int(js['lastBuild']['number'])
                    if prev is None or num > prev:
                        new_build = num
                        logger.info('Job %s triggered successfully: build #%s', job_name, new_build)
                        break
            except Exception:
                pass
        time.sleep(2)

    if new_build is None:
        logger.warning('Failed to get build number for job %s after %d seconds', job_name, queue_timeout)

    return {'job': job_name, 'build': new_build, 'queued': new_build is None}


def is_job_currently_running(base_url: str, job_name: str, user: str, token: str) -> bool:
    """
    Check if a Jenkins job is currently running (has an active build)

    Args:
        base_url: Jenkins base URL
        job_name: Name of the job to check
        user: Jenkins username
        token: Jenkins API token

    Returns:
        True if job has a build currently running, False otherwise
    """
    job_api = base_url.rstrip('/') + f'/job/{job_name}'

    try:
        # Query for lastBuild information including building status
        code, body = http_get(job_api + '/api/json?tree=lastBuild[number,building]', jenkins_auth_header(user, token))
        if code == 200 and body:
            js = json.loads(body.decode('utf-8', 'ignore'))
            if 'lastBuild' in js and js['lastBuild']:
                is_building = js['lastBuild'].get('building', False)
                if is_building:
                    build_num = js['lastBuild'].get('number', 'unknown')
                    logger.info('Job %s is currently running (build #%s)', job_name, build_num)
                    return True
        return False
    except Exception as e:
        logger.warning('Failed to check if job %s is running: %s', job_name, e)
        return False  # Assume not running if we can't determine status


def wait_for_build_complete(base_url: str, job_name: str, build_number: int,
                            user: str, token: str, timeout: int = DEFAULT_TIMEOUT) -> Dict[str, Any]:
    """
    Wait for a Jenkins build to complete

    Returns:
        Dictionary with 'result' and 'raw' keys
    """
    job_api = base_url.rstrip('/') + f'/job/{job_name}/{build_number}/api/json'
    deadline = time.time() + timeout

    logger.info('Waiting for %s #%s to complete (timeout: %d seconds)', job_name, build_number, timeout)

    while time.time() < deadline:
        code, body = http_get(job_api, jenkins_auth_header(user, token))
        if code == 200 and body:
            try:
                js = json.loads(body.decode('utf-8', 'ignore'))
                if js.get('result') is not None:
                    logger.info('Job %s #%s completed with result: %s', job_name, build_number, js.get('result'))
                    return {'result': js.get('result'), 'raw': js}
            except Exception:
                pass
        time.sleep(10)

    logger.error('Timeout waiting for %s #%s after %d seconds', job_name, build_number, timeout)
    return {'result': 'TIMEOUT'}


def fetch_artifact(base_url: str, job_name: str, build_number: int,
                  artifact_path: str, user: str, token: str, max_wait: int = 300) -> Optional[bytes]:
    """
    Fetch artifact from Jenkins with retry logic

    Args:
        max_wait: Maximum seconds to wait for artifact (default 300)
    """
    url = base_url.rstrip('/') + f'/job/{job_name}/{build_number}/artifact/{artifact_path.lstrip("/")}'

    logger.info('Fetching artifact: %s from %s #%s', artifact_path, job_name, build_number)

    max_retries = max_wait // 10
    for attempt in range(1, max_retries + 1):
        code, body = http_get(url, jenkins_auth_header(user, token), timeout=60)
        if code == 200 and body:
            logger.info('Successfully fetched artifact (%d bytes)', len(body))
            return body

        if attempt < max_retries:
            if attempt == 1:
                logger.info('Artifact not ready, waiting...')
            elif attempt % 6 == 0:
                logger.info('Still waiting for artifact... (%d seconds elapsed)', attempt * 10)
            time.sleep(10)

    logger.error('Failed to fetch artifact after %d seconds', max_wait)
    return None


class SimpleResultsHTMLParser(HTMLParser):
    """Simple HTML parser for results.html"""
    def __init__(self):
        super().__init__()
        self.in_row = False
        self.cells: List[str] = []
        self.current_data = ''
        self.rows: List[List[str]] = []

    def handle_starttag(self, tag, attrs):
        if tag.lower() == 'tr':
            self.in_row = True
            self.cells = []
        elif tag.lower() in ('td', 'th'):
            self.current_data = ''

    def handle_data(self, data):
        if self.in_row:
            self.current_data += data.strip() + ' '

    def handle_endtag(self, tag):
        if tag.lower() in ('td', 'th') and self.in_row:
            self.cells.append(self.current_data.strip())
            self.current_data = ''
        if tag.lower() == 'tr' and self.in_row:
            self.rows.append([c for c in self.cells if c])
            self.in_row = False


def parse_results_html(content: bytes) -> Dict[str, Any]:
    """
    Parse results.html to extract test summary

    Returns:
        Dictionary with 'summary', 'tests', and 'raw_rows' keys
    """
    try:
        html_text = content.decode('utf-8', 'ignore')
    except Exception:
        html_text = content.decode('latin1', 'ignore')

    parser = SimpleResultsHTMLParser()
    parser.feed(html_text)

    result = {
        'summary': {
            'tests_executed': 0,
            'failed': 0,
            'passed': 0,
            'not_run': 0,
            'pass_percentage': 0.0,
            'found': False
        },
        'tests': {},
        'raw_rows': parser.rows
    }

    # Parse summary section
    # Look for Test Engine Summary table (2nd table) with header: Test Engine | Board Name | Total Core Test | Passed Core Test | Failed/Partial Core Test | Not Run Core Test | Percentage
    test_engine_summary_found = False
    for i, row in enumerate(parser.rows):
        if len(row) >= 7:
            row_text = ' '.join(row).upper().replace(' ', '').replace('/', '')

            # Look for the Test Engine Summary table header
            if 'TESTENGINE' in row_text and 'BOARDNAME' in row_text and 'TOTALCORETEST' in row_text and 'PERCENTAGE' in row_text:
                # Next row should have the data
                if i + 1 < len(parser.rows):
                    data_row = parser.rows[i + 1]
                    if len(data_row) >= 7:
                        try:
                            # Row format: [TestEngine, BoardName, TotalCoreTest, PassedCoreTest, Failed/PartialCoreTest, NotRunCoreTest, Percentage]
                            # Indices:     [0,          1,          2,              3,               4,                     5,              6]
                            total = int(data_row[2])
                            passed = int(data_row[3])
                            failed = int(data_row[4])
                            not_run = int(data_row[5])
                            percentage_str = str(data_row[6]).replace('%', '').strip()
                            percentage = float(percentage_str)

                            result['summary']['tests_executed'] = total
                            result['summary']['passed'] = passed
                            result['summary']['failed'] = failed
                            result['summary']['not_run'] = not_run
                            result['summary']['pass_percentage'] = percentage
                            result['summary']['found'] = True
                            test_engine_summary_found = True
                            logger.info('Parsed Test Engine Summary: %d tests, %d passed, %.2f%% pass', total, passed, percentage)
                            break
                        except (ValueError, IndexError) as e:
                            logger.warning('Failed to parse Test Engine Summary row: %s', e)
                            continue

    # If Test Engine Summary not found, fall back to the first summary table
    if not test_engine_summary_found:
        logger.info('Test Engine Summary not found, trying Project Summary table as fallback')
        for i, row in enumerate(parser.rows):
            if len(row) >= 8:
                row_text = ' '.join(row).upper().replace(' ', '').replace('/', '')

                # Look for the Project summary table header
                if 'PROJECT' in row_text and 'PLATFORM' in row_text and 'TOTAL' in row_text and 'PERCENTAGE' in row_text:
                    # Next row should have the data
                    if i + 1 < len(parser.rows):
                        data_row = parser.rows[i + 1]
                        if len(data_row) >= 8:
                            try:
                                # Row format: [Project, Platform, ExecutionEngine, Total, Passed, Failed/Partial, Not_Run, Percentage]
                                # Indices:     [0,       1,        2,               3,     4,      5,              6,       7]
                                total = int(data_row[3])
                                passed = int(data_row[4])
                                failed = int(data_row[5])
                                not_run = int(data_row[6])
                                percentage_str = str(data_row[7]).replace('%', '').strip()
                                percentage = float(percentage_str)

                                result['summary']['tests_executed'] = total
                                result['summary']['passed'] = passed
                                result['summary']['failed'] = failed
                                result['summary']['not_run'] = not_run
                                result['summary']['pass_percentage'] = percentage
                                result['summary']['found'] = True
                                logger.info('Parsed Project Summary (fallback): %d tests, %d passed, %.2f%% pass', total, passed, percentage)
                                break
                            except (ValueError, IndexError) as e:
                                logger.warning('Failed to parse Project Summary row: %s', e)
                                continue

    # If summary not found, try to calculate from individual tests
    if not result['summary']['found']:
        logger.warning('Summary section not found in results.html')
        # Try to parse individual test cases
        test_case_patterns = [
            r'([A-Za-z0-9_\-\.\[\]\(\)\s]+?)\s+(PASS|PASSED|FAIL|FAILED|ERROR|SKIPPED|NOT[\s_]?RUN|NOTRUN)',
        ]

        for row in parser.rows:
            if len(row) < 2:
                continue
            joined = ' '.join(row)
            for pattern in test_case_patterns:
                m = re.search(pattern, joined, re.IGNORECASE)
                if m:
                    name = m.group(1).strip()
                    status = m.group(2).upper().replace(' ', '_')
                    if 'PASS' in status:
                        result['tests'][name] = 'PASSED'
                    elif 'FAIL' in status:
                        result['tests'][name] = 'FAILED'
                    elif 'ERROR' in status:
                        result['tests'][name] = 'ERROR'
                    else:
                        result['tests'][name] = 'NOT_RUN'
                    break

        # Calculate summary from tests
        if result['tests']:
            total = len(result['tests'])
            passed = sum(1 for s in result['tests'].values() if s == 'PASSED')
            failed = sum(1 for s in result['tests'].values() if s == 'FAILED')
            not_run = sum(1 for s in result['tests'].values() if s == 'NOT_RUN')

            result['summary']['tests_executed'] = total
            result['summary']['passed'] = passed
            result['summary']['failed'] = failed
            result['summary']['not_run'] = not_run
            result['summary']['pass_percentage'] = (passed / total * 100.0) if total > 0 else 0.0
            result['summary']['found'] = True
            logger.info('Calculated summary from test cases: %d tests, %.2f%% pass',
                       total, result['summary']['pass_percentage'])

    return result


def validate_test_results(test_data: Dict[str, Any], threshold: float = 90.0) -> Tuple[bool, List[str]]:
    """
    Validate test results against pass percentage threshold

    Args:
        test_data: Parsed test results
        threshold: Minimum pass percentage required (default 90.0)

    Returns:
        Tuple of (approved, reasons)
    """
    approved = True
    reasons = []

    if not test_data['summary']['found']:
        approved = False
        reasons.append('FAIL: Could not parse test summary from results.html')
        return approved, reasons

    pass_pct = test_data['summary']['pass_percentage']
    tests_executed = test_data['summary']['tests_executed']
    passed = test_data['summary']['passed']
    failed = test_data['summary']['failed']
    not_run = test_data['summary']['not_run']

    # Validate pass percentage
    if pass_pct >= threshold:
        reasons.append(f'PASS: Pass percentage {pass_pct:.2f}% meets threshold {threshold:.2f}%')
    else:
        approved = False
        reasons.append(f'FAIL: Pass percentage {pass_pct:.2f}% is below threshold {threshold:.2f}%')

    # Report test counts
    reasons.append(f'INFO: Tests executed: {tests_executed}, Passed: {passed}, Failed: {failed}, Not Run: {not_run}')

    return approved, reasons


def execute_git_promotion(repo_cfg: Dict[str, str], dry_run: bool = True) -> Dict[str, Any]:
    """
    Execute git promotion: rebase next onto prod, fast-forward merge, and push

    Args:
        repo_cfg: Repository configuration with clone_link, next_branch, prod_branch
        dry_run: If True, don't actually execute git commands

    Returns:
        Dictionary with promotion status
    """
    repo = repo_cfg.get('repo')
    clone = repo_cfg.get('clone_link')
    next_b = repo_cfg.get('next_branch')
    prod_b = repo_cfg.get('prod_branch')

    if not (clone and next_b and prod_b):
        return {'repo': repo, 'status': 'SKIPPED', 'reason': 'incomplete-config'}

    logger.info('Promoting %s: %s -> %s (dry_run=%s)', repo, next_b, prod_b, dry_run)

    if dry_run:
        return {'repo': repo, 'status': 'DRY_RUN'}

    tmp = os.path.join('/tmp', f'promo_{repo}')

    # Clean up any existing clone directory
    if os.path.exists(tmp):
        logger.info('Removing existing clone directory: %s', tmp)
        shutil.rmtree(tmp)

    cwd = os.getcwd()
    try:
        # Clone with next branch
        logger.info('Cloning repository %s...', clone)
        subprocess.check_call(['git', 'clone', '--no-tags', '--branch', next_b, clone, tmp])
        os.chdir(tmp)

        # Fetch the prod branch
        logger.info('Fetching branch %s...', prod_b)
        subprocess.check_call(['git', 'fetch', 'origin', prod_b])

        # Rebase next_branch onto prod_branch for linear history
        logger.info('Rebasing %s onto %s...', next_b, prod_b)
        subprocess.check_call(['git', 'rebase', f'origin/{prod_b}'])

        # Fast-forward prod_branch to match rebased next_branch
        subprocess.check_call(['git', 'checkout', prod_b])
        subprocess.check_call(['git', 'merge', '--ff-only', next_b])
        subprocess.check_call(['git', 'push', 'origin', prod_b])

        logger.info('Successfully promoted %s: %s -> %s', repo, next_b, prod_b)
        return {'repo': repo, 'status': 'PROMOTED'}

    except Exception as e:
        logger.exception('Promotion failed for %s', repo)
        return {'repo': repo, 'status': 'FAILED', 'error': str(e)}
    finally:
        # Always change back to original directory and clean up clone
        try:
            os.chdir(cwd)
        except Exception:
            pass
        try:
            if os.path.exists(tmp):
                logger.info('Cleaning up clone directory: %s', tmp)
                shutil.rmtree(tmp)
        except Exception as e:
            logger.warning('Failed to clean up clone directory %s: %s', tmp, e)


def promotion_flow(args: argparse.Namespace) -> int:
    """
    Main promotion workflow

    Returns:
        0 if promotion approved, 2 if rejected
    """
    logger.info('=' * 80)
    logger.info('ETHFW/ETHRTOS Promotion Workflow Started')
    logger.info('=' * 80)

    # Create artifacts directory if it doesn't exist
    os.makedirs(args.artifacts_dir, exist_ok=True)

    # Acquire exclusive lock to prevent multiple instances
    lock_file = os.path.join(args.artifacts_dir, '.promotion_lock')
    try:
        with ScriptLock(lock_file):
            return _promotion_flow_impl(args)
    except RuntimeError as e:
        logger.error(str(e))
        return 1


def _promotion_flow_impl(args: argparse.Namespace) -> int:
    """
    Internal implementation of promotion workflow

    Returns:
        0 if promotion approved, 2 if rejected
    """
    # Load configuration files
    jcfg = None
    if args.jenkins_config:
        if os.path.exists(args.jenkins_config):
            jcfg = load_ini(args.jenkins_config)
            logger.info('Loaded Jenkins config: %s', args.jenkins_config)
        else:
            logger.error('Jenkins config not found: %s', args.jenkins_config)
            return 2

    bcfg = None
    if args.branch_config:
        if os.path.exists(args.branch_config):
            bcfg = load_ini(args.branch_config)
            logger.info('Loaded branch config: %s', args.branch_config)
        else:
            logger.error('Branch config not found: %s', args.branch_config)
            return 2

    # Extract Jenkins configuration
    jinfo = {}
    if jcfg and 'JENKINS_SERVER_INFO' in jcfg:
        jinfo = dict(jcfg['JENKINS_SERVER_INFO'])

    user = jinfo.get('jenkins_user')
    token = jinfo.get('jenkins_api_token')
    base_url = jinfo.get('jenkins_url')
    test_base_url = jinfo.get('test_jenkins_url', base_url)
    test_user = jinfo.get('test_jenkins_user', user)
    test_token = jinfo.get('test_jenkins_api_token', token)
    nas_base = jinfo.get('nas_base_url')

    # Build job 1 (ETHFW)
    build_job = jinfo.get('build_job_name', 'ethrtos_ethfw_rtos')
    build_devices = [d.strip() for d in jinfo.get('build_devices', 'j721e').split(',')]
    test_jobs = [t.strip() for t in jinfo.get('test_jobs', '').split(',') if t.strip()]
    manifest_tag = jinfo.get('manifest_tag', 'REL.ETHFW.11.02.00.06')

    # Build job 2 (ENET LLD) - optional
    build_job_2 = jinfo.get('build_job_name_2', None)
    build_devices_2 = [d.strip() for d in jinfo.get('build_devices_2', '').split(',') if d.strip()]
    test_jobs_2 = [t.strip() for t in jinfo.get('test_jobs_2', '').split(',') if t.strip()]
    manifest_tag_2 = jinfo.get('manifest_tag_2', 'test')

    # Combine all build jobs
    build_jobs_config = [
        {'name': build_job, 'devices': build_devices, 'test_jobs': test_jobs, 'manifest_tag': manifest_tag}
    ]
    if build_job_2:
        build_jobs_config.append({'name': build_job_2, 'devices': build_devices_2, 'test_jobs': test_jobs_2, 'manifest_tag': manifest_tag_2})

    try:
        build_timeout = int(jinfo.get('build_timeout_seconds', DEFAULT_TIMEOUT))
    except Exception:
        build_timeout = DEFAULT_TIMEOUT

    try:
        queue_timeout = int(jinfo.get('queue_timeout_seconds', 1800))
    except Exception:
        queue_timeout = 1800

    try:
        test_timeout = int(jinfo.get('test_timeout_seconds', DEFAULT_TIMEOUT))
    except Exception:
        test_timeout = DEFAULT_TIMEOUT

    try:
        pass_threshold = float(jinfo.get('pass_percentage_threshold', 90.0))
    except Exception:
        pass_threshold = 90.0

    logger.info('Configuration:')
    for i, job_cfg in enumerate(build_jobs_config, 1):
        logger.info('  Build job %d: %s', i, job_cfg['name'])
        logger.info('    Devices: %s', ', '.join(job_cfg['devices']))
        logger.info('    Manifest tag: %s', job_cfg.get('manifest_tag', 'N/A'))
        logger.info('    Test jobs: %s', ', '.join(job_cfg['test_jobs']))
    logger.info('  Pass threshold: %.2f%%', pass_threshold)
    logger.info('  Branch to build: %s', args.branch)

    # ===== BUILD DEPENDENCY DICTIONARY =====
    # Key: job identifier, Value: list of jobs that must pass before this job can run
    dependencies = {}

    # Track which build job each build_id belongs to
    build_job_mapping = {}  # build_id -> build_job_name

    # Build jobs have no dependencies (can start immediately)
    for job_cfg in build_jobs_config:
        for device in job_cfg['devices']:
            build_id = f"build_{job_cfg['name']}_{device}"
            dependencies[build_id] = []
            build_job_mapping[build_id] = job_cfg['name']

    # Test jobs depend on their corresponding build job
    # Auto-generate mapping based on device names in test job names
    test_job_mapping = {}
    test_job_device_mapping = {}  # test_job -> device for later use

    for job_cfg in build_jobs_config:
        for test_job in job_cfg['test_jobs']:
            # Find matching device by checking if device name is in test job name
            matched_device = None
            for device in job_cfg['devices']:
                # Case-insensitive matching
                if device.lower() in test_job.lower():
                    matched_device = device
                    break

            if matched_device:
                build_id = f"build_{job_cfg['name']}_{matched_device}"
                test_job_mapping[test_job] = build_id
                test_job_device_mapping[test_job] = matched_device
                dependencies[test_job] = [build_id]
            else:
                logger.warning('Could not find matching device for test job: %s', test_job)
                dependencies[test_job] = []

    # ===== ADD DEVICE-LEVEL MUTUAL EXCLUSION =====
    # For test jobs that test the same device, create sequential dependencies
    # to prevent multiple test jobs for the same device from running simultaneously
    device_test_jobs = {}  # device -> list of test jobs for that device
    for test_job, device in test_job_device_mapping.items():
        if device not in device_test_jobs:
            device_test_jobs[device] = []
        device_test_jobs[device].append(test_job)

    # For devices with multiple test jobs, create sequential dependencies
    for device, jobs in device_test_jobs.items():
        if len(jobs) > 1:
            jobs.sort()  # Sort for deterministic ordering
            logger.info('Device %s has multiple test jobs - creating sequential dependencies:', device)
            for i in range(1, len(jobs)):
                # Test job at index i depends on test job at index i-1
                dependencies[jobs[i]].append(jobs[i-1])
                logger.info('  %s will wait for %s to complete', jobs[i], jobs[i-1])

    logger.info('')
    logger.info('Dependency mapping:')
    for job, deps in dependencies.items():
        logger.info('  %s depends on: %s', job, deps if deps else 'nothing')

    # Job tracking
    job_status = {}  # job_id -> {'triggered': bool, 'completed': bool, 'success': bool, 'build_num': int, 'result': dict}
    for job_id in dependencies:
        job_status[job_id] = {'triggered': False, 'completed': False, 'success': False, 'build_num': None, 'result': None}

    results = {
        'build_triggers': [],
        'build_results': [],
        'test_triggers': [],
        'test_results': [],
        'test_validations': []
    }

    # ===== MAIN EXECUTION LOOP =====
    logger.info('')
    logger.info('=' * 80)
    logger.info('Starting Dynamic Job Execution')
    logger.info('=' * 80)

    # Trigger initial jobs (builds with no dependencies)
    # TODO: Temporary - construct fsdk_link based on device (case-insensitive)
    fsdk_links = {
        'j721e': 'http://pdknas.dhcp.ti.com/docker_artifacts/JACINTO_RELEASE/latest/artifacts/output/webgen/exports/ti-processor-sdk-rtos-j721e-11_02_00_00.tar.gz',
        'j7200': 'http://pdknas.dhcp.ti.com/docker_artifacts/J7200_RELEASE/latest/artifacts/output/webgen/exports/ti-processor-sdk-rtos-j7200-11_02_00_00.tar.gz',
        'j784s4': 'http://pdknas.dhcp.ti.com/docker_artifacts/J784S4_RELEASE/latest/artifacts/output/webgen/exports/ti-processor-sdk-rtos-j784s4-11_02_00_00.tar.gz',
        'j721s2': 'http://pdknas.dhcp.ti.com/docker_artifacts/J721S2_RELEASE/latest/artifacts/output/webgen/exports/ti-processor-sdk-rtos-j721s2-11_02_00_00.tar.gz',
        'j742s2': 'http://pdknas.dhcp.ti.com/docker_artifacts/J742S2_RELEASE/latest/artifacts/output/webgen/exports/ti-processor-sdk-rtos-j742s2-11_02_00_00.tar.gz'
    }

    # Initialize success flags (will be updated if all jobs complete)
    all_builds_passed = False
    all_tests_passed = False

#   Toggle the below condition if you don't want to run the builds and tests and promote directly.
    if True:
        for job_cfg in build_jobs_config:
            job_name = job_cfg['name']
            job_manifest_tag = job_cfg.get('manifest_tag', 'REL.ETHFW.11.02.00.06')
    
            for device in job_cfg['devices']:
                build_id = f"build_{job_name}_{device}"
    
                build_params = {
                    'product_family': device,
                    'release_build': jinfo.get('release_build', 'true'),
                    'BRANCH': args.branch,
                    'fsdk_link': fsdk_links.get(device.lower(), ''),
                    'manifest_tag': job_manifest_tag
                }
    
                # Set enet_build flag for enet_lld builds
                if 'enet_lld' in job_name.lower():
                    build_params['enet_build'] = 'true'
                else:
                    build_params['enet_build'] = 'false'
    
                logger.info('Triggering build job "%s" for device: %s (manifest_tag: %s)', job_name, device, job_manifest_tag)
                trigger_info = trigger_jenkins_job(base_url, job_name, build_params, user, token, queue_timeout)
                trigger_info['device'] = device
                trigger_info['job_id'] = build_id
                trigger_info['build_job_name'] = job_name
    
                job_status[build_id]['triggered'] = True
                job_status[build_id]['build_num'] = trigger_info['build']
    
                results['build_triggers'].append(trigger_info)
                logger.info('Build job %s marked as triggered with build #%s', build_id, trigger_info['build'])

        # Main loop: monitor running jobs and trigger ready ones
        while True:
            # Check if all jobs are done
            all_done = all(status['completed'] for status in job_status.values())
            if all_done:
                logger.info('All jobs completed!')
                break
    
            # Log pending jobs
            pending_jobs = [job_id for job_id, status in job_status.items() if not status['completed']]
            if pending_jobs:
                logger.info('Jobs still pending: %s', ', '.join(pending_jobs))
    
            # Check status of triggered but not completed jobs
            for job_id, status in job_status.items():
                if status['triggered'] and not status['completed'] and status['build_num'] is not None:
                    # Determine if this is a build or test job
                    if job_id.startswith('build_'):
                        jenkins_url = base_url
                        jenkins_user = user
                        jenkins_token = token
                        # Extract build job name from build_id
                        jenkins_job = build_job_mapping.get(job_id, build_job)
                    else:
                        jenkins_url = test_base_url
                        jenkins_user = test_user
                        jenkins_token = test_token
                        jenkins_job = job_id  # test job name is the job_id itself
    
                    # Check if job completed (non-blocking check)
                    job_api = jenkins_url.rstrip('/') + f'/job/{jenkins_job}/{status["build_num"]}/api/json'
                    code, body = http_get(job_api, jenkins_auth_header(jenkins_user, jenkins_token))
    
                    if code == 200 and body:
                        try:
                            js = json.loads(body.decode('utf-8', 'ignore'))
                            if js.get('result') is not None:
                                # Mark job as completed (prevent re-processing)
                                if status['completed']:
                                    # Already marked as completed, skip
                                    continue
    
                                status['completed'] = True
                                status['success'] = (js.get('result') == 'SUCCESS')
                                status['result'] = js
    
                                logger.info('Job %s #%s completed: %s (marking as completed)', job_id, status['build_num'], js.get('result'))
    
                                # Check if job failed
                                if js.get('result') != 'SUCCESS':
                                   logger.error('Job %s #%s FAILED with result: %s', job_id, status['build_num'], js.get('result'))
                                   logger.error('Stopping promotion due to job failure')
                                   return 2
    
                                # Store result
                                if job_id.startswith('build_'):
                                    # Extract device from build_id (format: build_jobname_device)
                                    build_job_name = build_job_mapping.get(job_id, '')
                                    device = job_id.replace(f'build_{build_job_name}_', '')
                                    results['build_results'].append({
                                        'device': device,
                                        'job': jenkins_job,
                                        'build': status['build_num'],
                                        'result': js.get('result')
                                    })
                                else:
                                    # Test job completed successfully - validate results immediately
                                    results['test_results'].append({
                                        'job': job_id,
                                        'build': status['build_num'],
                                        'result': js.get('result')
                                    })
    
                                    # Fetch and validate results.html
                                    logger.info('Fetching results.html from %s #%s', job_id, status['build_num'])
                                    artifact_content = fetch_artifact(test_base_url, job_id, status['build_num'],
                                                                    'artifacts/results.html', test_user, test_token)
    
                                    if not artifact_content:
                                        logger.error('Failed to fetch results.html from %s #%s', job_id, status['build_num'])
                                        logger.error('Stopping promotion due to missing test results')
                                        return 2
    
                                    # Parse results.html
                                    logger.info('Parsing results.html from %s #%s', job_id, status['build_num'])
                                    test_data = parse_results_html(artifact_content)
    
                                    # Validate against threshold
                                    approved, reasons = validate_test_results(test_data, pass_threshold)
    
                                    validation_result = {
                                        'job': job_id,
                                        'build': status['build_num'],
                                        'approved': approved,
                                        'reasons': reasons,
                                        'test_summary': test_data['summary']
                                    }
                                    results['test_validations'].append(validation_result)
    
                                    logger.info('Test validation for %s #%s:', job_id, status['build_num'])
                                    for reason in reasons:
                                        logger.info('  %s', reason)
    
                                    if not approved:
                                        logger.error('Test validation failed for %s #%s', job_id, status['build_num'])
                                        logger.error('Stopping promotion due to test validation failure')
                                        return 2
                        except Exception:
                            pass
    
            # Check for jobs ready to trigger
            for job_id, deps in dependencies.items():
                status = job_status[job_id]
    
                # Skip if already triggered or completed
                if status['triggered'] or status['completed']:
                    continue
    
                # Check if all dependencies are completed and successful
                can_trigger = True
                for dep in deps:
                    dep_status = job_status.get(dep)
                    if not dep_status or not dep_status['completed'] or not dep_status['success']:
                        can_trigger = False
                        break
    
                if can_trigger:
                    # Trigger this job
                    if job_id.startswith('build_'):
                        # This shouldn't happen as builds have no dependencies and are triggered initially
                        pass
                    else:
                        # Additional safety check: verify this test job hasn't been triggered before
                        # by checking if it appears in test_triggers
                        already_triggered = any(t['job'] == job_id for t in results['test_triggers'])
                        if already_triggered:
                            logger.warning('Test job %s was already triggered in this execution, skipping duplicate trigger', job_id)
                            status['triggered'] = True  # Mark as triggered to prevent future attempts
                            continue
    
                        # Check if job is already running in Jenkins (even if not triggered by us)
                        if is_job_currently_running(test_base_url, job_id, test_user, test_token):
                            logger.warning('Test job %s is already running in Jenkins, skipping trigger for this iteration', job_id)
                            # Do NOT mark as triggered - we didn't trigger it
                            # Just skip this iteration and check again later
                            continue
    
                        # Trigger test job - need to construct RTOS_BINS from the build job
                        test_params = {
                            'TEST_LABEL': jinfo.get('TEST_LABEL', 'NIGHTLY'),
                            'TEST_TYPE': jinfo.get('TEST_TYPE', 'FULL'),
                            'RELEASE_VERSION': jinfo.get('RELEASE_VERSION', '0x_0x_0x_0x'),
                            'INSTALLER_BUILD_ID': jinfo.get('INSTALLER_BUILD_ID', 'latest'),
                        }
    
                        if jcfg and 'TEST_PARAMETERS' in jcfg:
                            for key, value in jcfg['TEST_PARAMETERS'].items():
                                if key not in test_params:
                                    test_params[key] = value
    
                        # Add JIRA filter if configured for this test job
                        if jcfg and 'JIRA_FILTERS' in jcfg:
                            if job_id in jcfg['JIRA_FILTERS']:
                                test_params['JIRA_FILTER'] = jcfg['JIRA_FILTERS'][job_id]
                                logger.info('Using JIRA filter for %s: %s', job_id, jcfg['JIRA_FILTERS'][job_id])
    
                        # Construct RTOS_BINS URL from the build job
                        # Find the corresponding build job for this test
                        build_dep = deps[0] if deps else None
                        if build_dep:
                            build_status = job_status.get(build_dep)
                            if build_status and build_status['build_num']:
                                # Extract build job name and device from build dependency
                                # Format: build_jobname_device
                                build_job_name = build_job_mapping.get(build_dep, build_job)
                                device = build_dep.replace(f'build_{build_job_name}_', '')
                                build_num = build_status['build_num']
    
                                # Construct RTOS_BINS URL based on build job type
                                if 'enet_lld' in build_job_name.lower():
                                    # For ENET LLD builds
                                    rtos_bins_url = f"{nas_base}/enet/{build_job_name}_{build_num}/artifacts/output/enet-lld-rtos-{device}-11_02_00_00-binary_only.tar.gz"
                                else:
                                    # For ETHFW builds
                                    rtos_bins_url = f"{nas_base}/ethfw/{build_job_name}_{build_num}/artifacts/output/ethfw-rtos-{device}-11_02_00_00-binary_only.tar.gz"
    
                                test_params['RTOS_BINS'] = rtos_bins_url
    
                                logger.info('RTOS_BINS for %s: %s (from build %s)', job_id, rtos_bins_url, build_dep)
    
                        logger.info('Triggering test job: %s (dependencies satisfied)', job_id)
                        trigger_info = trigger_jenkins_job(test_base_url, job_id, test_params, test_user, test_token, queue_timeout)
                        trigger_info['job'] = job_id  # Ensure job name is included
    
                        # Mark as triggered BEFORE appending to results to prevent race conditions
                        status['triggered'] = True
                        status['build_num'] = trigger_info['build']
    
                        results['test_triggers'].append(trigger_info)
    
                        logger.info('Test job %s marked as triggered with build #%s', job_id, trigger_info['build'])
    
            # Sleep before next poll
            time.sleep(10)
    
        # ===== ALL JOBS COMPLETED SUCCESSFULLY =====
        logger.info('')
        logger.info('=' * 80)
        logger.info('All jobs completed successfully!')
        logger.info('=' * 80)
        logger.info('Build results:')
        for br in results['build_results']:
            logger.info('  %s (%s): %s', br['device'], br['job'], br['result'])
        logger.info('Test results:')
        for tr in results['test_results']:
            logger.info('  %s: %s', tr['job'], tr['result'])
    
        # Calculate success flags
        all_builds_passed = all(br['result'] == 'SUCCESS' for br in results['build_results'])
        all_tests_passed = all(tr['result'] == 'SUCCESS' for tr in results['test_results'])

    # ===== PROMOTION DECISION =====
    logger.info('')
    logger.info('=' * 80)
    logger.info('PROMOTION DECISION')
    logger.info('=' * 80)

    approved = True
    if args.force_promote:
        logger.warning('FORCE PROMOTE enabled - bypassing validation checks')

    logger.info('Final Decision:  APPROVED')
    logger.info('=' * 80)

    # ===== EXECUTE PROMOTION (if approved) =====
    promo_actions = []
    if approved and not args.dry_run:
        logger.info('')
        logger.info('=' * 80)
        logger.info('PHASE 7: Executing Branch Promotions')
        logger.info('=' * 80)

        if bcfg:
            for section in bcfg.sections():
                repo = dict(bcfg[section])
                repo['repo'] = section
                logger.info('Promoting repository: %s', section)
                action = execute_git_promotion(repo, dry_run=args.dry_run)
                promo_actions.append(action)
                logger.info('Promotion result for %s: %s', section, action.get('status'))
        else:
            logger.warning('No branch configuration provided - skipping git promotions')
    elif approved and args.dry_run:
        logger.info('')
        logger.info('DRY RUN MODE - Skipping actual git promotions')

    # ===== WRITE SUMMARY REPORT =====
    summary = {
        'approved': approved,
        'all_builds_passed': all_builds_passed,
        'all_tests_passed': all_tests_passed,
        'build_triggers': results['build_triggers'],
        'build_results': results['build_results'],
        'test_triggers': results['test_triggers'],
        'test_results': results['test_results'],
        'test_validations': results['test_validations'],
        'promotions': promo_actions
    }

    out_dir = args.artifacts_dir
    os.makedirs(out_dir, exist_ok=True)
    out_file = os.path.join(out_dir, 'promotion_summary.json')

    with open(out_file, 'w') as f:
        json.dump(summary, f, indent=2)

    logger.info('')
    logger.info('Promotion summary written to: %s', out_file)
    logger.info('')

    # Check if promotion actions failed (if we attempted promotion)
    if approved and not args.dry_run and promo_actions:
        failed_promotions = [act for act in promo_actions if act.get('status') == 'FAILED']
        if failed_promotions:
            logger.error('')
            logger.error('=' * 80)
            logger.error('PROMOTION FAILED')
            logger.error('=' * 80)
            for act in failed_promotions:
                error_msg = act.get('error', 'Unknown error')
                logger.error('  Repository: %s', act.get('repo'))
                logger.error('  Error: %s', error_msg)
            logger.error('=' * 80)
            logger.error('')
            return 1  # Return error code when promotion fails

    return 0 if approved else 2


def build_parser() -> argparse.ArgumentParser:
    """Build argument parser"""
    p = argparse.ArgumentParser(description='ETHFW/ETHRTOS Promotion Script')
    p.add_argument('--jenkins-config',
                   default='ethfw_jenkins.cfg',
                   help='Path to Jenkins configuration file')
    p.add_argument('--branch-config',
                   default='ethfw_branch.cfg',
                   help='Path to branch configuration file')
    p.add_argument('--branch',
    # TODO: Update the branch name below
                   default='ethfw_cicd',
                   help='Branch to build and test from (default: next)')
    p.add_argument('--artifacts-dir',
                   default='artifacts',
                   help='Directory to store artifacts and reports')
    p.add_argument('--dry-run',
                   action='store_true',
                   help='Dry run mode - do not execute git promotions')
    p.add_argument('--force-promote',
                   action='store_true',
                   help='Force promotion even if validation fails')
    return p


def main():
    """Main entry point"""
    parser = build_parser()
    args = parser.parse_args()

    # Setup logging to file
    os.makedirs(args.artifacts_dir, exist_ok=True)
    log_file = os.path.join(args.artifacts_dir, 'promotion.log')
    fh = logging.FileHandler(log_file)
    fh.setFormatter(logging.Formatter('%(asctime)s %(levelname)s: %(message)s'))
    logger.addHandler(fh)

    logger.info('Arguments: %s', vars(args))

    try:
        rc = promotion_flow(args)
        sys.exit(rc)
    except Exception as e:
        logger.exception('Fatal error in promotion flow')
        sys.exit(1)


if __name__ == '__main__':
    main()
