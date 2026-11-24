"""Build the Project D interpreter and compare its output with the supplied examples."""

import argparse
from pathlib import Path
import platform
import re
import shlex
import shutil
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parent
BUILD = ROOT / '.build' / platform.system().lower()
COMPILER = shutil.which('g++') or shutil.which('clang++')
EXE = '.exe' if sys.platform == 'win32' else ''
INTERPRETER = BUILD / ('dinterpret' + EXE)

LEXER = ['lexer/src/lexer.cpp', 'lexer/src/token.cpp']
PARSER = LEXER + ['syntax/src/parser.cpp', 'syntax/src/ast.cpp']
SEMANTIC = PARSER + ['semantic/src/semantic.cpp']
OPTIMIZER = SEMANTIC + ['semantic/src/optimizer.cpp']
RUNTIME = OPTIMIZER + [
    'interpreter/src/interpreter.cpp', 'interpreter/src/expressions.cpp',
    'interpreter/src/value.cpp', 'interpreter/src/value_operations.cpp',
    'interpreter/src/runtime_objects.cpp', 'interpreter/src/main.cpp',
]
LEGACY_TARGETS = {
    'dlang': LEXER + ['lexer/src/main.cpp'],
    'dparse': PARSER + ['syntax/src/main.cpp'],
    'dsemantic': OPTIMIZER + ['semantic/src/main.cpp'],
    'lexer_tests': LEXER + ['lexer/tests/lexer_tests.cpp'],
    'parser_tests': PARSER + ['syntax/tests/parser_tests.cpp'],
    'semantic_tests': SEMANTIC + ['semantic/tests/semantic_tests.cpp'],
    'optimizer_tests': OPTIMIZER + ['semantic/tests/optimizer_tests.cpp'],
}
LEGACY_CLI = {'lexer': 'dlang', 'syntax': 'dparse', 'semantic': 'dsemantic'}
TEAM_PROGRAMS = ROOT / 'project-d-examples/project_d_example_programs.md'


def run(arguments, *, verbose, cwd=ROOT, suite=False):
    command = [str(arg) for arg in arguments]
    if verbose:
        print('+ ' + shlex.join(command), flush=True)
    if suite and not verbose:
        result = subprocess.run(command, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        if result.returncode:
            sys.stderr.buffer.write(result.stdout)
            result.check_returncode()
    else:
        subprocess.run(command, cwd=cwd, check=True)


def check_example(source, stdout, stderr, status, verbose):
    for options in ([], ['--optimize']):
        command = [str(INTERPRETER), *options, source.name]
        if verbose:
            print('+ ' + shlex.join(command) + ' in ' + str(source.parent), flush=True)
        try:
            result = subprocess.run(command, cwd=source.parent, capture_output=True,
                                    text=True, encoding='utf-8', timeout=10)
        except subprocess.TimeoutExpired:
            raise SystemExit(f'{source} {options}: timed out')
        actual = (result.returncode, result.stdout, result.stderr)
        expected = (status, stdout, stderr)
        if actual != expected:
            raise SystemExit(f'{source} {options}\n'
                             f'  expected (exit, stdout, stderr): {expected!r}\n'
                             f'  received (exit, stdout, stderr): {actual!r}')


def check_file_examples(verbose):
    sources = sorted((ROOT / 'interpreter/examples').glob('*.d'))
    if not sources:
        raise SystemExit('No interpreter examples found')
    for source in sources:
        out = source.with_suffix('.out')
        err = source.with_suffix('.err')
        if not out.is_file() and not err.is_file():
            raise SystemExit(f'{source}: missing expected output (.out or .err)')
        stdout = out.read_text(encoding='utf-8') if out.is_file() else ''
        stderr = err.read_text(encoding='utf-8') if err.is_file() else ''
        check_example(source, stdout, stderr, int(err.is_file()), verbose)
    return len(sources)


def check_team_examples(verbose):
    sections = re.split(r'(?m)^## (?=\d+\. )', TEAM_PROGRAMS.read_text(encoding='utf-8'))[1:]
    code_block = re.compile(r'^```text\n(.*?)^```[ \t]*$', re.MULTILINE | re.DOTALL)
    with tempfile.TemporaryDirectory() as directory:
        for number, section in enumerate(sections, 1):
            title, body = section.split('\n', 1)
            before, separator, after = body.partition('### Expected output')
            program, output = code_block.search(before), code_block.search(after)
            if not (title.startswith(f'{number}. ') and separator and program and output):
                raise SystemExit(f'{TEAM_PROGRAMS}: missing program or expected output for {title}')
            source = Path(directory) / f'example_{number:02}.d'
            source.write_text(program[1], encoding='utf-8')
            check_example(source, output[1], '', 0, verbose)
    if len(sections) != 15:
        raise SystemExit(f'{TEAM_PROGRAMS}: expected 15 programs, found {len(sections)}')
    return len(sections)


def build(targets, verbose):
    BUILD.mkdir(parents=True, exist_ok=True)
    includes = ['-I' + str(ROOT / name / 'include')
                for name in ('lexer', 'syntax', 'semantic', 'interpreter')]
    flags = ['-std=c++17', '-O2', '-Wall', '-Wextra', '-Wpedantic', *includes]
    objects = {}
    for name, sources in targets.items():
        for source in sources:
            if source not in objects:
                target = BUILD / Path(source.replace('/', '_')).with_suffix('.o')
                run([COMPILER, *flags, '-c', ROOT / source, '-o', target], verbose=verbose)
                objects[source] = target
        run([COMPILER, *(objects[source] for source in sources), '-o', BUILD / (name + EXE)],
            verbose=verbose)


def require_legacy_checkout():
    files = [TEAM_PROGRAMS, *(ROOT / stage / 'tests/test_integration.py' for stage in LEGACY_CLI)]
    for sources in LEGACY_TARGETS.values():
        files.extend(ROOT / source for source in sources)
    for file in files:
        if not file.is_file():
            raise SystemExit(f'--all requires the full course checkout (missing {file.relative_to(ROOT)}).')
    if sys.platform == 'win32':
        raise SystemExit('The original frontend suites require Unix line endings. Use WSL for --all.')


def check_legacy_suites(verbose):
    for name in ('lexer_tests', 'parser_tests', 'semantic_tests', 'optimizer_tests'):
        run([BUILD / name], verbose=verbose, suite=True)
    for repo, binary in LEGACY_CLI.items():
        run([sys.executable, ROOT / repo / 'tests/test_integration.py', BUILD / binary],
            cwd=ROOT / repo, verbose=verbose, suite=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument('--build-only', action='store_true', help='build dinterpret and exit')
    mode.add_argument('--all', action='store_true', help='also run the original course suites')
    parser.add_argument('-v', '--verbose', action='store_true', help='show commands and test logs')
    args = parser.parse_args()
    targets = {'dinterpret': RUNTIME}
    if args.all:
        require_legacy_checkout()
        targets.update(LEGACY_TARGETS)
    if COMPILER is None:
        raise SystemExit('A C++17 compiler (g++ or clang++) is required.')
    build(targets, args.verbose)
    print(f'Built {INTERPRETER.relative_to(ROOT)}', flush=True)
    if args.build_only:
        return
    count = check_file_examples(args.verbose)
    print(f'Passed {count} examples with and without optimization.', flush=True)
    if args.all:
        check_legacy_suites(args.verbose)
        team = check_team_examples(args.verbose)
        print(f'Passed the original frontend suites and {team} team programs.')


if __name__ == '__main__':
    try:
        main()
    except subprocess.CalledProcessError as error:
        print(f'Failed: {shlex.join(error.cmd)}', file=sys.stderr)
        raise SystemExit(error.returncode)
