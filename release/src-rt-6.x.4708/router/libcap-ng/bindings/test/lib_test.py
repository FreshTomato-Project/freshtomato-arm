#!/usr/bin/env python3

import sys
import capng


def get_last_cap():
    last = capng.CAP_LAST_CAP
    try:
        with open('/proc/sys/kernel/cap_last_cap', 'r') as f:
            last = int(f.readline())
    except OSError:
        pass
    return last


def main():
    last = get_last_cap()

    print("Doing basic bit tests...")
    capng.capng_clear(capng.CAPNG_SELECT_BOTH)
    if capng.capng_have_capabilities(capng.CAPNG_SELECT_BOTH) != capng.CAPNG_NONE:
        print("Failed clearing capabilities")
        sys.exit(1)

    # capng_save_state/capng_restore_state are not available in python bindings
    capng.capng_fill(capng.CAPNG_SELECT_BOTH)
    if capng.capng_have_capabilities(capng.CAPNG_SELECT_BOTH) != capng.CAPNG_FULL:
        print("Failed filling capabilities")
        sys.exit(1)

    text = capng.capng_print_caps_numeric(capng.CAPNG_PRINT_BUFFER,
                                         capng.CAPNG_SELECT_CAPS)
    if len(text) < 80 and last > 30:
        last = 30

    print("Doing advanced bit tests for %d capabilities..." % last)
    for i in range(last + 1):
        capng.capng_clear(capng.CAPNG_SELECT_BOTH)
        rc = capng.capng_update(capng.CAPNG_ADD, capng.CAPNG_EFFECTIVE, i)
        if rc:
            print("Failed update test 1")
            sys.exit(1)
        rc = capng.capng_have_capability(capng.CAPNG_EFFECTIVE, i)
        if rc <= capng.CAPNG_NONE:
            print("Failed have capability test 1")
            capng.capng_print_caps_numeric(capng.CAPNG_PRINT_STDOUT,
                                           capng.CAPNG_SELECT_CAPS)
            sys.exit(1)
        if capng.capng_have_capabilities(capng.CAPNG_SELECT_CAPS) != capng.CAPNG_PARTIAL:
            print("Failed have capabilities test 1")
            capng.capng_print_caps_numeric(capng.CAPNG_PRINT_STDOUT,
                                           capng.CAPNG_SELECT_CAPS)
            sys.exit(1)
        if capng.CAP_LAST_CAP > 31:
            rc = capng.capng_update(capng.CAPNG_ADD, capng.CAPNG_BOUNDING_SET, i)
            if rc:
                print("Failed bset update test 2")
                sys.exit(1)
            rc = capng.capng_have_capability(capng.CAPNG_BOUNDING_SET, i)
            if rc <= capng.CAPNG_NONE:
                print("Failed bset have capability test 2")
                capng.capng_print_caps_numeric(capng.CAPNG_PRINT_STDOUT,
                                               capng.CAPNG_SELECT_BOTH)
                sys.exit(1)
            if capng.capng_have_capabilities(capng.CAPNG_SELECT_BOUNDS) != capng.CAPNG_PARTIAL:
                print("Failed bset have capabilities test 2")
                capng.capng_print_caps_numeric(capng.CAPNG_PRINT_STDOUT,
                                               capng.CAPNG_SELECT_BOTH)
                sys.exit(1)
        text = capng.capng_print_caps_text(capng.CAPNG_PRINT_BUFFER,
                                           capng.CAPNG_EFFECTIVE)
        name = capng.capng_capability_to_name(i)
        if text != name:
            print("Failed print text comparison")
            print("%s != %s" % (text, name))
            sys.exit(1)
        capng.capng_fill(capng.CAPNG_SELECT_BOTH)
        rc = capng.capng_update(capng.CAPNG_DROP, capng.CAPNG_EFFECTIVE, i)
        if rc:
            print("Failed update test 3")
            sys.exit(1)
        if capng.capng_have_capabilities(capng.CAPNG_SELECT_CAPS) != capng.CAPNG_PARTIAL:
            print("Failed have capabilities test 3")
            capng.capng_print_caps_numeric(capng.CAPNG_PRINT_STDOUT,
                                           capng.CAPNG_SELECT_CAPS)
            sys.exit(1)
        rc = capng.capng_update(capng.CAPNG_ADD, capng.CAPNG_EFFECTIVE, i)
        if rc:
            print("Failed update test 4")
            sys.exit(1)
        if capng.capng_have_capabilities(capng.CAPNG_SELECT_CAPS) != capng.CAPNG_FULL:
            print("Failed have capabilities test 4")
            capng.capng_print_caps_numeric(capng.CAPNG_PRINT_STDOUT,
                                           capng.CAPNG_SELECT_CAPS)
            sys.exit(1)

    capng.capng_clear(capng.CAPNG_SELECT_BOTH)
    rc = capng.capng_updatev(capng.CAPNG_ADD, capng.CAPNG_EFFECTIVE,
                             capng.CAP_CHOWN, capng.CAP_FOWNER, capng.CAP_KILL, -1)
    if rc:
        print("Failed updatev test")
        sys.exit(1)
    rc = (capng.capng_have_capability(capng.CAPNG_EFFECTIVE, capng.CAP_CHOWN) and
          capng.capng_have_capability(capng.CAPNG_EFFECTIVE, capng.CAP_FOWNER) and
          capng.capng_have_capability(capng.CAPNG_EFFECTIVE, capng.CAP_KILL))
    if not rc:
        print("Failed have updatev capability test")
        capng.capng_print_caps_numeric(capng.CAPNG_PRINT_STDOUT,
                                       capng.CAPNG_SELECT_CAPS)
        sys.exit(1)

    sys.exit(0)


if __name__ == '__main__':
    main()
