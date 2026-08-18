# FreshTomato Platform and Build Symbols

This document records the platform/build symbol semantics discovered while
unifying the ARM and MIPS FreshTomato trees.

Its main purpose is to prevent accidental substitutions between symbols that
currently select similar targets but belong to different build layers or have
different meanings.

## Supported platform families

| Family | Repository / tree | Router platform symbols |
|---|---|---|
| ARM | ARM branch | `TCONFIG_BCMARM=y`, `TCONFIG_RTNPLUS=y` |
| MIPS RT | `mips-master`, `release/src-rt` | `TCONFIG_MIPS_RT=y` |
| MIPS RT-N | `mips-RT-AC`, `release/src-rt` | `TCONFIG_MIPS_RTN=y`, `TCONFIG_BLINK=y`, `TCONFIG_RTNPLUS=y` |
| MIPS RT-AC | `mips-RT-AC`, `release/src-rt-6.x` | `TCONFIG_MIPS_RTAC=y`, `TCONFIG_BLINK=y`, `TCONFIG_RTNPLUS=y`, `TCONFIG_BCMWL6=y` |

`TCONFIG_MIPS` is derived from the three MIPS families.

`TCONFIG_RTNPLUS` is a generation/capability selector:

```text
ARM        -> y
MIPS RT    -> n
MIPS RT-N  -> y
MIPS RT-AC -> y
```

It must be derived from the platform family, not set manually by targets.

## Symbol layers

Do not treat symbols from different rows as interchangeable.

| Layer | Examples | Meaning |
|---|---|---|
| Top-level/bootstrap identity | `TARGET_PLATFORM`, `TARGET_VARIANT` | Selects the build tree/toolchain before router Kconfig is necessarily available |
| Vendor/SDK build identity | `CONFIG_BCMWL6`, `CONFIG_BCMWL6A`, `CONFIG_BCM7`, `CONFIG_BCM714` | Low-level Broadcom SDK/driver build selection |
| Router platform identity | `TCONFIG_BCMARM`, `TCONFIG_MIPS_RT`, `TCONFIG_MIPS_RTN`, `TCONFIG_MIPS_RTAC`, `TCONFIG_MIPS` | Router/Kconfig platform family |
| Router generation/capability | `TCONFIG_RTNPLUS`, `TCONFIG_BLINK`, `TCONFIG_BCMWL6`, `TCONFIG_BCMWL6A` | Derived router-side selectors; semantics are narrower than simple platform aliases |
| Vendor compile macros | `__CONFIG_DHDAP__` | Driver/vendor source compile-time identity |
| Router feature symbols | `TCONFIG_DHDAP`, `TCONFIG_DPSTA`, `TCONFIG_GMAC3`, etc. | Router feature selection |
| Raw SDK/Make variables | `AC3200`, `AC5300`, `DPSTA` | Still consumed directly by vendor/top-level Makefiles |

## Platform matrix

The important platform selectors currently behave as follows:

| Symbol | ARM | MIPS RT | MIPS RT-N | MIPS RT-AC | Meaning |
|---|:---:|:---:|:---:|:---:|---|
| `TCONFIG_BCMARM` | y | n | n | n | Router-side ARM platform |
| `TCONFIG_MIPS_RT` | n | y | n | n | Legacy MIPS RT |
| `TCONFIG_MIPS_RTN` | n | n | y | n | MIPS RT-N |
| `TCONFIG_MIPS_RTAC` | n | n | n | y | MIPS RT-AC |
| `TCONFIG_RTNPLUS` | y | n | y | y | RT-N generation or newer |
| `TCONFIG_BLINK` | n | n | y | y | MIPS RT-N / RT-AC selector |
| `TCONFIG_BCMWL6` | y | n | n | y | Router-side WL6 capability |
| `TCONFIG_BCMWL6A` | y | n | n | n | Router-side ARM WL6A capability |
| `CONFIG_BCMWL6` | y | n | n | y | Low-level WL6 SDK/build identity |
| `CONFIG_BCMWL6A` | y | n | n | n | Low-level ARM WL6A/build identity |

`TCONFIG_BCM7` and `TCONFIG_BCM714` are ARM SDK-family subsets, not generic
"newer platform" selectors.

## Critical rule: equal truth tables do not imply equal semantics

Some expressions currently evaluate to the same target set:

```c
defined(TCONFIG_BLINK) || defined(TCONFIG_BCMARM)
defined(CONFIG_BCMWL6) || defined(TCONFIG_BLINK)
defined(CONFIG_BCMWL6A) || defined(TCONFIG_BLINK)
```

Today all three can select ARM + RT-N + RT-AC.

They are **not** interchangeable.

Use `TCONFIG_RTNPLUS` only when the code really means:

> RT-N generation or newer, regardless of the concrete SDK/driver family.

Do not replace `CONFIG_BCMWL6` or `CONFIG_BCMWL6A` merely because the resulting
truth table happens to match `TCONFIG_RTNPLUS`.

Examples:

```c
#ifdef TCONFIG_RTNPLUS
/* functionality shared by ARM, RT-N and RT-AC */
#endif
```

is appropriate for genuine RT-N+ behavior.

This:

```c
#ifdef CONFIG_BCMWL6
/* code tied to the WL6 SDK/driver */
#endif
```

must remain tied to `CONFIG_BCMWL6`.

Likewise:

```c
#ifdef CONFIG_BCMWL6A
/* ARM-specific vendor/SDK path */
#endif
```

must not be rewritten as a generic ARM or RT-N+ test without checking the
consumer and build stage.

## `TCONFIG_BLINK`

`TCONFIG_BLINK` is currently derived from:

```text
TCONFIG_MIPS_RTN || TCONFIG_MIPS_RTAC
```

It therefore means MIPS RT-N/RT-AC, not generic RT-N+.

Keep it when code intentionally distinguishes the MIPS RT-N/RT-AC path from
ARM or legacy RT.

Do not mechanically replace all `TCONFIG_BLINK` uses with
`TCONFIG_RTNPLUS`.

## `CONFIG_BCMWL6A` and ARM

`CONFIG_BCMWL6A` is an active low-level ARM build identity.

Known consumers include:

```text
src/btools/libfoo.pl
src/target.mak
```

For example, `libfoo.pl` detects ARM through the environment variable
`CONFIG_BCMWL6A`.

There is also router/build glue using `TCONFIG_BCMWL6A`, such as
`src/ctools/Makefile`.

These are different layers. Do not remove one just because the other exists.

## Raw Make variables that are still active

Full-tree auditing confirmed direct Makefile consumers of variables such as:

```text
AC3200
AC5300
DPSTA
```

Examples include top-level SDK Makefiles and `wl/config/wl.mk`.

Their router-side `TCONFIG_*` equivalents do not make the raw variables
automatically obsolete.

## DHDAP

Keep these concepts separate:

```text
__CONFIG_DHDAP__  -> vendor/driver compile macro
TCONFIG_DHDAP     -> router-side feature/config symbol
```

Code may intentionally accept either:

```c
#if defined(__CONFIG_DHDAP__) || defined(TCONFIG_DHDAP)
```

Do not collapse them without auditing the vendor driver build.

## `common.mak` and early build stages

Some Makefiles execute before `router/.config` is guaranteed to have been
loaded.

Do not introduce dependencies on derived `TCONFIG_*` symbols into an early
bootstrap stage unless the include/order is verified.

For early build identity prefer the values exported by the target layer, such
as:

```text
TARGET_PLATFORM
TARGET_VARIANT
CONFIG_BCMWL6
CONFIG_BCMWL6A
CONFIG_BCM7
CONFIG_BCM714
```

when that is the existing build contract.

## MIPS branch unification policy

The long-term goal is for common source files to be text-identical between
`mips-master` and `mips-RT-AC`.

`mips-master` should therefore contain the full RT / RT-N / RT-AC source
superset, with branch-specific behavior selected by explicit symbols.

In particular:

- RT-N/RT-AC-only code should be moved into `mips-master` behind the correct
  `TCONFIG_*` or `CONFIG_*` guard before relying on merges.
- A first merge after introducing such guards may still conflict because Git
  sees the old branch-specific code and the new conditional superset as
  competing edits.
- Resolve that first merge to the common superset version.
- Afterward, keep the common files byte-identical across branches where
  practical.

Do not solve a merge conflict by dropping RT-N/RT-AC code merely because it is
absent from legacy RT.

## ABI-sensitive enums and constants

Model IDs, hardware IDs and LED numbers can be externally significant.

When editing `shared.h`:

- preserve historical numeric values;
- do not insert model/hardware enum entries in the middle if their numeric
  value is persisted or otherwise relied upon;
- verify RT, RT-N/RT-AC and ARM separately;
- keep platform-specific LED numbering stable.

Current LED layout:

```text
MIPS RT:     LED_USB=7,              LED_COUNT=8
MIPS RT-N+:  LED_USB=7, LED_5G=8,    LED_COUNT=9
ARM:         LED_USB=7, LED_USB3=8, LED_5G=9, ...
```

## `defaults.c`

The unified `shared/defaults.c` intentionally has different consumers on ARM
and MIPS.

ARM uses Broadcom's larger `struct nvram_tuple`.

MIPS keeps the compact two-pointer `defaults_t` for the nvram utility, and
builds the full defaults tables with `NVRAM_DEFAULTS_FULL`. The normal MIPS
`libshared` path keeps the historical empty `router_defaults[]` stub.

Do not change that ownership/model as part of unrelated source unification.

RT-AC-specific `nvram_default_get()` / name-fixup behavior is tied to
`CONFIG_BCMWL6`, not to generic `TCONFIG_RTNPLUS`.

## Known full-tree audit findings

The following old identity mechanisms were audited and no active project
consumer was found:

```text
ARM_SDK
MIPS_FAMILY
SDK6MIPS
NO_BLINK
$(ARM)
$(BCM7)
$(BCM714)
$(MIPS_FAMILY)
$(ARM_SDK)
```

`CONFIG_RT` requires care during grep audits: old MIPS kernel/driver sources
also use an unrelated local `CONFIG_RT` macro ("Retries"). Such hits are not
evidence that the old FreshTomato branch selector is still alive.

`TCONFIG_MIPSR2` has had low-level consumers in kernel/build code in the past.
Do not remove or rename low-level architecture/compiler symbols based only on
a `router/` search.

## Audit before changing platform symbols

Before removing, renaming or redefining a platform/build symbol:

1. Search the **entire branch**, not only `router/` or the kernel.
2. Search C/C++ sources, headers, Makefiles, shell, Perl and generated/build
   helper scripts.
3. Search both `TCONFIG_*` and raw `CONFIG_*` / Make-variable forms.
4. Distinguish definitions from consumers.
5. Check vendor/kernel trees separately from router code.
6. Compare ARM, MIPS RT and the combined MIPS RT-N/RT-AC branch.
7. Interpret name collisions manually; do not rely on grep counts alone.
8. Build representative targets after the change.
9. For common files, verify the resulting file is identical between branches
   when identical text is the intended end state.

For deprecated symbols, prefer exact-name matching. Avoid broad expressions
such as `CONFIG_RT.*`, because they also match unrelated kernel options such
as `CONFIG_RTC_*`, `CONFIG_RT_MUTEXES`, `CONFIG_RT_GROUP_SCHED`, etc.

## Rule of thumb

When reviewing a conditional, ask what it describes:

```text
platform?
generation?
SDK?
wireless driver?
vendor compile mode?
router feature?
```

Choose the symbol that expresses that meaning, not merely one that happens to
select the same targets today.
