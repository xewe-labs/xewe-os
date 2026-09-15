#!/usr/bin/env bash
set -euo pipefail

# setup.sh — assemble XeWe OS: choose modules, install them into src/modules/, install the build
# toolchain into build/, and run the build environment setup.
#
# Usage:
#   ./setup.sh                            interactive: pick modules in a checklist
#   ./setup.sh --modules wifi,scheduler   non-interactive (required modules are added)
#   ./setup.sh --modules all
#
# Options:
#   --modules <list|all>        modules to install (slugs), instead of the checklist
#   --modules-index <url|file>  module registry list (default: repositories.txt of xewe-labs/xewe-os-modules)
#   --modules-source <dir>      ignore the registry; use local xewe-os-module-* clones in <dir>
#   --modules-ref <ref>         branch or tag to pull modules from git (default: main)
#   --toolchain-source <dir>    install the toolchain from a local clone
#   --toolchain-ref <ref>       branch or tag of xewe-os-build-toolchain (default: main)
#   --skip-toolchain            don't install the toolchain
#   --skip-build-setup          don't run the toolchain's build/scripts/<platform>/setup.sh
#   --text                      numbered menu instead of the whiptail checklist
#   -h, --help
#
# Environment: XEWE_MODULES_INDEX (same as --modules-index), XEWE_GITHUB_OWNER (default
# xewe-labs) and XEWE_GIT_BASE_URL (default https://github.com/<owner>) for the toolchain,
# GITHUB_TOKEN (optional, for reading private repos).
#
# The registry (github.com/xewe-labs/xewe-os-modules) lists module repos, one per line; each
# repo's module.properties gives its slug, description and requirements. Anyone can add a module
# with a pull request to that list.
#
# Each module's src/<Folder>/ is copied to src/modules/<Folder>/ (no .git, no module .ino or
# scripts). setup.sh owns src/modules/ as a whole, including the generated Modules.h, .gitignore
# and modules.lock (what was installed, from where). From the toolchain repo only scripts/ and
# tools/code_formatter/ are copied into build/, next to the project's build data; the toolchain's
# build/scripts/<platform>/setup.sh then does the rest of the build setup (libraries, venv,
# build_config, build/.gitignore). Re-running setup.sh replaces all of it.

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="${PROJECT_ROOT}/src"
BUILD_DIR="${PROJECT_ROOT}/build"
MODULES_DIR="${SRC_DIR}/modules"
LOCK_FILE="${MODULES_DIR}/modules.lock"

GITHUB_OWNER="${XEWE_GITHUB_OWNER:-xewe-labs}"
GIT_BASE_URL="${XEWE_GIT_BASE_URL:-https://github.com/${GITHUB_OWNER}}"
DEFAULT_MODULES_INDEX="https://raw.githubusercontent.com/xewe-labs/xewe-os-modules/main/repositories.txt"
MODULE_PREFIX="xewe-os-module-"
TOOLCHAIN_REPO="xewe-os-build-toolchain"
TOOLCHAIN_PATHS="scripts tools/code_formatter"   # parts of the toolchain repo installed into build/

modules_arg=""
modules_index="${XEWE_MODULES_INDEX:-${DEFAULT_MODULES_INDEX}}"
modules_source=""
modules_ref="main"
toolchain_source=""
toolchain_ref="main"
skip_toolchain=0
skip_build_setup=0
text_menu=0

die()  { echo "❌ $*" >&2; exit 1; }
info() { echo "➜ $*"; }
usage() { sed -n '4,37p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit "${1:-0}"; }

# value of <key> in a key=value file (empty when missing)
prop() {
  local line
  line="$(grep -m1 "^$2=" "$1" 2>/dev/null || true)"
  printf '%s' "${line#*=}"
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --modules)          modules_arg="${2:?missing value}"; shift 2 ;;
    --modules-index)    modules_index="${2:?missing value}"; shift 2 ;;
    --modules-source)   modules_source="${2:?missing value}"; shift 2 ;;
    --modules-ref)      modules_ref="${2:?missing value}"; shift 2 ;;
    --toolchain-source) toolchain_source="${2:?missing value}"; shift 2 ;;
    --toolchain-ref)    toolchain_ref="${2:?missing value}"; shift 2 ;;
    --skip-toolchain)   skip_toolchain=1; shift ;;
    --skip-build-setup) skip_build_setup=1; shift ;;
    --text)             text_menu=1; shift ;;
    -h|--help)          usage 0 ;;
    *) echo "unknown argument: $1" >&2; usage 1 ;;
  esac
done

if [[ -n "${modules_source}" ]]; then
  [[ -d "${modules_source}" ]] || die "--modules-source not found: ${modules_source}"
  modules_source="$(cd "${modules_source}" && pwd)"
fi
if [[ -n "${toolchain_source}" ]]; then
  [[ -d "${toolchain_source}" ]] || die "--toolchain-source not found: ${toolchain_source}"
  toolchain_source="$(cd "${toolchain_source}" && pwd)"
fi

command -v git >/dev/null 2>&1 || die "git is required"
export GIT_TERMINAL_PROMPT=0     # fail instead of asking for a login when a repo is missing
WORK="$(mktemp -d)"
trap 'rm -rf "${WORK}"' EXIT
mkdir -p "${WORK}/modules" "${WORK}/manifests"

# ---------------------------------------------------------------------------
# module sources
# ---------------------------------------------------------------------------
curl_auth() {
  if [[ -n "${GITHUB_TOKEN:-}" ]]; then
    curl -fsSL -H "Authorization: Bearer ${GITHUB_TOKEN}" "$@"
  else
    curl -fsSL "$@"
  fi
}

# The registry is loaded once into ${INDEX_FILE}: "slug|entry" per module, where entry is a git
# URL or a local folder, and each module's module.properties is kept in ${WORK}/manifests/<slug>.
INDEX_FILE="${WORK}/index.txt"
: > "${INDEX_FILE}"

expand_home() { case "$1" in "~"|"~/"*) printf '%s' "${HOME}${1#\~}" ;; *) printf '%s' "$1" ;; esac; }

# https://github.com/<owner>/<repo>[.git] -> raw URL of module.properties at the requested ref
github_manifest_url() {
  local path
  case "$1" in https://github.com/*) ;; *) return 1 ;; esac
  path="${1#https://github.com/}"
  path="${path%/}"
  path="${path%.git}"
  [[ "${path}" == */* && "${path}" != */*/* ]] || return 1
  printf 'https://raw.githubusercontent.com/%s/%s/module.properties' "${path}" "${modules_ref}"
}

load_index() {
  local list="${WORK}/repositories.txt" entry n=0 manifest slug clone raw
  if [[ -f "$(expand_home "${modules_index}")" ]]; then
    cp "$(expand_home "${modules_index}")" "${list}"
  else
    command -v curl >/dev/null 2>&1 || die "curl is required to read the module registry"
    curl_auth "${modules_index}" -o "${list}" 2>/dev/null \
      || die "cannot read the module registry at ${modules_index}; pass --modules-index <file> or --modules-source <dir>"
  fi

  while IFS= read -r entry || [[ -n "${entry}" ]]; do
    entry="${entry%%#*}"
    entry="$(printf '%s' "${entry}" | tr -d '[:space:]')"
    [[ -n "${entry}" ]] || continue
    n=$((n + 1))
    manifest="${WORK}/manifests/entry-${n}"
    clone=""

    if [[ -d "$(expand_home "${entry}")" ]]; then
      entry="$(cd "$(expand_home "${entry}")" && pwd)"
      cp "${entry}/module.properties" "${manifest}" 2>/dev/null || die "registry entry ${entry} has no module.properties"
    elif raw="$(github_manifest_url "${entry}")" && curl_auth "${raw}" -o "${manifest}" 2>/dev/null; then
      :
    else
      clone="${WORK}/modules/entry-${n}"
      git clone --quiet --depth 1 --branch "${modules_ref}" "${entry}" "${clone}" 2>/dev/null \
        || die "cannot read registry entry ${entry} (ref ${modules_ref})"
      cp "${clone}/module.properties" "${manifest}" 2>/dev/null || die "registry entry ${entry} has no module.properties"
    fi

    slug="$(prop "${manifest}" slug)"
    [[ "${slug}" =~ ^[a-z0-9][a-z0-9-]*$ ]] || die "registry entry ${entry}: invalid or missing slug '${slug}'"
    if grep -q "^${slug}|" "${INDEX_FILE}"; then
      die "registry lists module '${slug}' twice: $(grep "^${slug}|" "${INDEX_FILE}" | cut -d'|' -f2-) and ${entry}"
    fi
    mv "${manifest}" "${WORK}/manifests/${slug}"
    [[ -z "${clone}" ]] || mv "${clone}" "${WORK}/modules/${slug}"
    echo "${slug}|${entry}" >> "${INDEX_FILE}"
  done < "${list}"

  [[ -s "${INDEX_FILE}" ]] || die "the module registry at ${modules_index} lists no modules"
}

index_entry() { grep "^$1|" "${INDEX_FILE}" | head -1 | cut -d'|' -f2-; }

# folder holding the module repo (local folder, or a shallow clone into WORK); prints its path
module_repo() {
  local slug="$1" dir entry
  if [[ -n "${modules_source}" ]]; then
    dir="${modules_source}/${MODULE_PREFIX}${slug}"
    [[ -f "${dir}/module.properties" ]] || die "module '${slug}' not found: expected ${dir}/module.properties"
    printf '%s' "${dir}"
    return 0
  fi

  entry="$(index_entry "${slug}")"
  [[ -n "${entry}" ]] || die "module '${slug}' is not in the registry (${modules_index})"
  if [[ -d "${entry}" ]]; then
    dir="${entry}"
  else
    dir="${WORK}/modules/${slug}"
    if [[ ! -d "${dir}" ]]; then
      git clone --quiet --depth 1 --branch "${modules_ref}" "${entry}" "${dir}" \
        || die "cannot clone ${entry} (ref ${modules_ref})"
    fi
  fi
  [[ -f "${dir}/module.properties" ]] || die "module '${slug}' (${entry}) has no module.properties"
  printf '%s' "${dir}"
}

# slugs available for the checklist, in registry order
catalog() {
  local d
  if [[ -n "${modules_source}" ]]; then
    for d in "${modules_source}/${MODULE_PREFIX}"*; do
      [[ -f "${d}/module.properties" ]] && prop "${d}/module.properties" slug && echo
    done
  else
    cut -d'|' -f1 "${INDEX_FILE}"
  fi
}

# one-line description for the checklist
describe() {
  if [[ -n "${modules_source}" ]]; then
    prop "${modules_source}/${MODULE_PREFIX}$1/module.properties" description
  else
    prop "${WORK}/manifests/$1" description
  fi
}

installed_slugs() {
  [[ -f "${LOCK_FILE}" ]] || return 0
  grep -v '^#' "${LOCK_FILE}" | cut -d'|' -f1
}

# ---------------------------------------------------------------------------
# selection
# ---------------------------------------------------------------------------
select_modules() {
  local slugs installed slug desc status choice
  slugs="$(catalog)"
  [[ -n "${slugs}" ]] || die "no modules found"
  installed="$(installed_slugs | tr '\n' ' ')"

  if [[ ${text_menu} -eq 0 ]] && command -v whiptail >/dev/null 2>&1; then
    local args=()
    for slug in ${slugs}; do
      desc="$(describe "${slug}")"
      status=ON
      if [[ -n "${installed}" ]]; then
        case " ${installed} " in *" ${slug} "*) status=ON ;; *) status=OFF ;; esac
      fi
      args+=("${slug}" "${desc:-${slug}}" "${status}")
    done
    choice="$(whiptail --title "XeWe OS modules" --separate-output \
      --checklist "Space toggles, Enter confirms. Modules required by your selection are added automatically." \
      20 90 10 "${args[@]}" 3>&1 1>&2 2>&3)" || die "cancelled"
    printf '%s' "${choice}" | tr '\n' ' '
    return 0
  fi

  echo "Available modules:" >&2
  local i=0 numbered=""
  for slug in ${slugs}; do
    i=$((i + 1))
    numbered="${numbered} ${slug}"
    case " ${installed} " in *" ${slug} "*) status="x" ;; *) status=" " ;; esac
    printf '  %2d. [%s] %-16s %s\n' "${i}" "${status}" "${slug}" "$(describe "${slug}")" >&2
  done
  read -rp "Modules to install (numbers or names, 'all'; Enter keeps [x]): " choice
  if [[ -z "${choice}" ]]; then
    [[ -n "${installed}" ]] || die "nothing selected"
    printf '%s' "${installed}"
  elif [[ "${choice}" == "all" ]]; then
    printf '%s' "${slugs}" | tr '\n' ' '
  else
    local word out=""
    for word in ${choice//,/ }; do
      if [[ "${word}" =~ ^[0-9]+$ ]]; then
        slug="$(echo ${numbered} | cut -d' ' -f"${word}")"
        [[ -n "${slug}" ]] || die "no module number ${word}"
        out="${out} ${slug}"
      else
        out="${out} ${word}"
      fi
    done
    printf '%s' "${out}"
  fi
}

if [[ -z "${modules_source}" ]]; then
  load_index
fi

if [[ "${modules_arg}" == "all" ]]; then
  requested="$(catalog | tr '\n' ' ')"
elif [[ -n "${modules_arg}" ]]; then
  requested="${modules_arg//,/ }"
elif [[ -t 0 ]]; then
  requested="$(select_modules)"
else
  die "no terminal for the module checklist; pass --modules <list|all>"
fi
[[ -n "${requested// /}" ]] || die "no modules selected"

# ---------------------------------------------------------------------------
# resolve required modules (dependencies first)
# ---------------------------------------------------------------------------
ORDER=""
VISITING=""
visit() {
  local slug="$1" dir deps dep
  case " ${ORDER} " in *" ${slug} "*) return 0 ;; esac
  case " ${VISITING} " in *" ${slug} "*) die "dependency cycle through module '${slug}'" ;; esac
  dir="$(module_repo "${slug}")"
  VISITING="${VISITING} ${slug}"
  deps="$(prop "${dir}/module.properties" depends_modules)"
  for dep in ${deps//,/ }; do
    case " ${requested} " in *" ${dep} "*) ;; *) info "${slug} requires ${dep}; adding it" ;; esac
    visit "${dep}"
  done
  VISITING="${VISITING/ ${slug}/}"
  ORDER="${ORDER} ${slug}"
}
for slug in ${requested}; do visit "${slug}"; done

# ---------------------------------------------------------------------------
# fetch the toolchain first, so a failed download leaves src/modules/ untouched
# ---------------------------------------------------------------------------
if [[ ${skip_toolchain} -eq 0 ]]; then
  staged="${WORK}/toolchain"
  if [[ -n "${toolchain_source}" ]]; then
    [[ -f "${toolchain_source}/scripts/common/paths.sh" ]] \
      || die "${toolchain_source} does not look like ${TOOLCHAIN_REPO}"
    mkdir -p "${staged}"
    (cd "${toolchain_source}" && tar --exclude=./.git -cf - .) | (cd "${staged}" && tar -xf -)
    toolchain_desc="${toolchain_source}"
  else
    git clone --quiet --depth 1 --branch "${toolchain_ref}" "${GIT_BASE_URL}/${TOOLCHAIN_REPO}.git" "${staged}" \
      || die "cannot clone ${GIT_BASE_URL}/${TOOLCHAIN_REPO}.git (ref ${toolchain_ref})"
    toolchain_desc="${GIT_BASE_URL}/${TOOLCHAIN_REPO}.git @ ${toolchain_ref}"
  fi
  for path in ${TOOLCHAIN_PATHS}; do
    [[ -e "${staged}/${path}" ]] || die "${toolchain_desc} has no ${path}"
  done
fi

# ---------------------------------------------------------------------------
# install modules into src/modules/ (staged, then swapped in)
# ---------------------------------------------------------------------------
staged_modules="${WORK}/src-modules"
mkdir -p "${staged_modules}"

lock="# generated by setup.sh: slug|folder|source|ref|commit"
includes=""
declarations=""
for slug in ${ORDER}; do
  dir="$(module_repo "${slug}")"
  manifest="${dir}/module.properties"
  folder="$(prop "${manifest}" folder)"
  include="$(prop "${manifest}" include)"
  declare_line="$(prop "${manifest}" declare)"
  [[ -n "${folder}" && "${folder}" != *"/"* && "${folder}" != "."* ]] || die "${MODULE_PREFIX}${slug}: invalid folder '${folder}'"
  [[ -d "${dir}/src/${folder}" ]] || die "${MODULE_PREFIX}${slug}: src/${folder} not found"
  [[ ! -e "${staged_modules}/${folder}" ]] || die "${MODULE_PREFIX}${slug}: folder '${folder}' is used by another module"
  [[ -n "${include}" && -n "${declare_line}" ]] || die "${MODULE_PREFIX}${slug}: module.properties needs include= and declare="

  cp -R "${dir}/src/${folder}" "${staged_modules}/${folder}"
  rm -rf "${staged_modules}/${folder}/.git"

  if [[ -n "${modules_source}" ]]; then
    source_desc="${dir}"
    ref="local"
  else
    source_desc="$(index_entry "${slug}")"
    ref="${modules_ref}"
    [[ ! -d "${source_desc}" ]] || ref="local"
  fi
  commit="$(git -C "${dir}" rev-parse --verify -q HEAD 2>/dev/null || echo "-")"
  lock="${lock}
${slug}|${folder}|${source_desc}|${ref}|${commit}"
  includes="${includes}#include \"${include#src/}\"
"
  declarations="${declarations}${declare_line}
"
  info "installed src/modules/${folder}  (${MODULE_PREFIX}${slug} $(prop "${manifest}" version))"
done

cat > "${staged_modules}/Modules.h" <<EOF
// Generated by setup.sh; do not edit. Re-run setup.sh to change modules.
// Included by xewe-os.ino after the ModuleController \`os\`; modules are declared in dependency order.
#pragma once

${includes}
${declarations}
EOF

cat > "${staged_modules}/.gitignore" <<EOF
# Generated by setup.sh: src/modules/ is installed, not part of this repository.
*
EOF

printf '%s\n' "${lock}" > "${staged_modules}/modules.lock"

rm -rf "${MODULES_DIR:?}"
mkdir -p "${SRC_DIR}"
mv "${staged_modules}" "${MODULES_DIR}"

# ---------------------------------------------------------------------------
# toolchain into build/
# ---------------------------------------------------------------------------
if [[ ${skip_toolchain} -eq 0 ]]; then
  for path in ${TOOLCHAIN_PATHS}; do
    rm -rf "${BUILD_DIR:?}/${path}"
    mkdir -p "$(dirname "${BUILD_DIR}/${path}")"
    cp -R "${staged}/${path}" "${BUILD_DIR}/${path}"
  done
  info "installed the toolchain into build/  (${toolchain_desc})"
fi

# ---------------------------------------------------------------------------
# build environment
# ---------------------------------------------------------------------------
echo
echo "Modules: $(echo ${ORDER})"
if [[ ${skip_build_setup} -eq 1 ]]; then
  echo "Build setup skipped; run it later with build/scripts/<mac|linux>/setup.sh"
  exit 0
fi
[[ -d "${BUILD_DIR}/scripts" ]] || die "build/scripts is missing; run without --skip-toolchain"

case "$(uname -s)" in
  Darwin) platform=mac ;;
  Linux)  platform=linux ;;
  *) echo "Run build\\scripts\\windows\\setup.ps1 in PowerShell to finish."
     exit 0 ;;
esac
info "running build/scripts/${platform}/setup.sh"
exec "${BUILD_DIR}/scripts/${platform}/setup.sh"
