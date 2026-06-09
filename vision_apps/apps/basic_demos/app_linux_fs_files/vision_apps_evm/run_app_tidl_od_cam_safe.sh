#!/bin/sh

CFG_FILE="app_od_cam_safe.cfg"
APP_BIN="/opt/vision_apps/vx_app_tidl_od_cam_safe.out"
LOG_FILE="app_tidl_od_cam_safe.log"

core=""
mode=""
test_option=""
test_name=""
expected_msg_1=""
expected_msg_2=""
exec_time_seconds=10

print_help() {
    echo "Usage:"
    echo "  $0"
    echo "  $0 --core <r5f|a72|c7x> --rc [--exec-time <seconds>]"
    echo "  $0 --core <r5f|a72|c7x> --psm [--exec-time <seconds>]"
    echo "  $0 --help"
    echo
    echo "Options:"
    echo "  --core <r5f|a72|c7x>     Select target core"
    echo "  --rc                     Reciprocal Comparison test (normal run, no error injection)"
    echo "  --psm                    Program Sequence Monitoring test (normal run, no error injection)"
    echo "  --exec-time <seconds>    Time to let the app run before sending 'x' (default: 10)"
    echo "  --help, -h               Show this help"
    echo
    echo "Default behavior:"
    echo "  If no options are provided, the application runs in normal mode"
    echo "  (test_option = 0), meaning all mechanisms are active in their"
    echo "  standard configuration without forced error injection."
    echo
    echo "Test behavior:"
    echo "  In test modes (--rc / --psm), the application runs normally and"
    echo "  the test passes if NO expected error messages are detected in the output."
    echo
    echo "Examples:"
    echo "  $0"
    echo "  $0 --core r5f --rc"
    echo "  $0 --core a72 --psm --exec-time 15"
}

set_option() {
    sed -i "s/^[[:space:]]*test_option[[:space:]].*/test_option      $1/" "$CFG_FILE"
}

print_line() {
    echo "============================================================"
}

run_app() {
    : > "$LOG_FILE" || {
        echo "Error: failed to create $LOG_FILE"
        exit 1
    }

    ( sleep "$exec_time_seconds"; echo "x" ) | "$APP_BIN" --cfg "$CFG_FILE" 2>&1 | tee "$LOG_FILE"
}

run_test_case() {
    print_line
    echo "Starting test case: $test_name"
    echo "Config file : $CFG_FILE"
    echo "Log file    : $LOG_FILE"
    echo "Exec time   : $exec_time_seconds seconds"
    print_line

    set_option "$test_option" || {
        echo "Error: failed to update $CFG_FILE"
        exit 1
    }

    run_app
    app_status=$?

    echo
    print_line
    echo "Application finished with status: $app_status"
    echo "Checking log for unexpected error messages..."
    print_line

    test_failed=0

    if grep -q "$expected_msg_1" "$LOG_FILE"; then
        echo "Unexpected message found: $expected_msg_1"
        test_failed=1
    fi

    if [ -n "$expected_msg_2" ]; then
        if grep -q "$expected_msg_2" "$LOG_FILE"; then
            echo "Unexpected message found: $expected_msg_2"
            test_failed=1
        fi
    fi

    if [ "$test_failed" -eq 0 ]; then
        echo "No unexpected messages found."
        print_line
        echo "TEST RESULT: PASSED"
    else
        print_line
        echo "TEST RESULT: FAILED"
    fi

    set_option 0
    exit "$app_status"
}

while [ $# -gt 0 ]; do
    case "$1" in
        --core)
            if [ -n "$2" ]; then
                core="$2"
                shift 2
            else
                echo "Error: --core requires a value"
                exit 1
            fi
            ;;
        --rc)
            mode="rc"
            shift
            ;;
        --psm)
            mode="psm"
            shift
            ;;
        --exec-time)
            if [ -n "$2" ]; then
                exec_time_seconds="$2"
                shift 2
            else
                echo "Error: --exec-time requires a value"
                exit 1
            fi
            ;;
        --help|-h)
            print_help
            exit 0
            ;;
        *)
            echo "Error: Unknown argument: $1"
            echo "Use --help for usage."
            exit 1
            ;;
    esac
done

case "$exec_time_seconds" in
    *[!0-9]*|"")
        echo "Error: --exec-time must be a positive integer"
        exit 1
        ;;
esac

if [ -n "$core" ] || [ -n "$mode" ]; then
    if [ -z "$core" ] || [ -z "$mode" ]; then
        echo "Error: both --core and one of --rc/--psm must be provided together"
        exit 1
    fi

    case "$core:$mode" in
        r5f:rc)
            test_option=1
            test_name="Reciprocal Comparison on R5F"
            expected_msg_1="ERROR: AEWB node comparison failed"
            expected_msg_2=""
            ;;
        a72:rc)
            test_option=2
            test_name="Reciprocal Comparison on A72"
            expected_msg_1="ERROR: PreProc node comparison failed"
            expected_msg_2=""
            ;;
        c7x:rc)
            test_option=3
            test_name="Reciprocal Comparison on C7X"
            expected_msg_1="ERROR: DrawDet node comparison failed"
            expected_msg_2=""
            ;;
        r5f:psm)
            test_option=4
            test_name="Program Sequence Monitoring on R5F"
            expected_msg_1="ERROR: AEWB tmp too large"
            expected_msg_2="ERROR: node timestamp order invalid"
            ;;
        a72:psm)
            test_option=5
            test_name="Program Sequence Monitoring on A72"
            expected_msg_1="ERROR: PREPROC tmp too large"
            expected_msg_2="ERROR: node timestamp order invalid"
            ;;
        c7x:psm)
            test_option=6
            test_name="Program Sequence Monitoring on C7X"
            expected_msg_1="ERROR: DRAWDET tmp too large"
            expected_msg_2="ERROR: node timestamp order invalid"
            ;;
        *)
            echo "Error: invalid combination --core $core and --$mode"
            exit 1
            ;;
    esac

    run_test_case
fi

"$APP_BIN" --cfg "$CFG_FILE"
app_status=$?

set_option 0
exit "$app_status"