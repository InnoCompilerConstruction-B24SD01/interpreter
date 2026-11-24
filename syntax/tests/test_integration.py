"""Exercise the parser CLI with the supplied Project D programs."""

from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest


BINARY = Path(sys.argv.pop(1) if len(sys.argv) > 1 else "./dparse").resolve()
TESTS = Path(__file__).resolve().parent
FIXTURES = TESTS / "fixtures"
PROJECT_EXAMPLES = TESTS.parents[1] / "project-d-examples" / "project_d_example_programs.md"
SYNTAX_EXAMPLES = TESTS.parent / "examples"
CODE_BLOCK = re.compile(
    r"^(?P<indent> *)```text\n(?P<code>.*?)^(?P=indent)```[ \t]*$",
    re.MULTILINE | re.DOTALL,
)
AST_LINE = re.compile(r'(?:  )*[A-Za-z]+(?: "(?:[^"\\]|\\.)*")?')


def supplied_programs():
    document = PROJECT_EXAMPLES.read_text(encoding="utf-8")
    headings = list(re.finditer(r"^## ([0-9]+)\. [^\n]+$", document, re.MULTILINE))
    if [int(heading[1]) for heading in headings] != list(range(1, 16)):
        raise ValueError("Expected exactly the 15 numbered Project D programs")

    programs = {}
    for index, heading in enumerate(headings):
        end = headings[index + 1].start() if index + 1 < len(headings) else len(document)
        blocks = list(CODE_BLOCK.finditer(document[heading.end():end]))
        if len(blocks) < 2:
            raise ValueError("Each numbered program must have source and expected output")
        programs["example_{:02d}".format(int(heading[1]))] = blocks[0]["code"]

    for fixture in sorted((FIXTURES / "specification").glob("*.d")):
        programs["spec_" + fixture.stem] = fixture.read_text(encoding="utf-8")
    return programs


PROGRAMS = supplied_programs()


class ParserCliTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix="dlang parser tests ")
        self.addCleanup(self.directory.cleanup)
        self.source_path = Path(self.directory.name) / "source file.d"

    def run_cli(self, *arguments):
        return subprocess.run([str(BINARY), *map(str, arguments)], capture_output=True, timeout=10)

    def run_source(self, source):
        self.source_path.write_bytes(source.encode("utf-8") if isinstance(source, str) else source)
        return self.run_cli(self.source_path)

    def check_valid(self, source):
        result = self.run_source(source)
        self.assertEqual(result.returncode, 0, result.stderr.decode("utf-8"))
        self.assertEqual(result.stderr, b"")
        lines = result.stdout.decode("utf-8").splitlines()
        self.assertTrue(lines)
        self.assertEqual(lines[0], "Program")
        for line in lines:
            self.assertIsNotNone(AST_LINE.fullmatch(line), "Invalid AST line: " + line)
        return result.stdout.decode("utf-8")

    def test_all_supplied_programs(self):
        self.assertEqual(len(PROGRAMS), 18)
        for name, source in PROGRAMS.items():
            with self.subTest(name=name):
                self.check_valid(source)

    def test_expression_ast(self):
        self.assertEqual(
            self.check_valid("var result := 1 + 2 * 3\nprint result\n"),
            "Program\n"
            "  VariableDeclaration\n"
            "    Variable \"result\"\n"
            "      BinaryExpression \"+\"\n"
            "        IntegerLiteral \"1\"\n"
            "        BinaryExpression \"*\"\n"
            "          IntegerLiteral \"2\"\n"
            "          IntegerLiteral \"3\"\n"
            "  PrintStatement\n"
            "    Identifier \"result\"\n",
        )

    def test_checked_in_input_output_examples(self):
        sources = sorted(SYNTAX_EXAMPLES.glob("*.d"))
        self.assertGreaterEqual(len(sources), 3)
        for source in sources:
            with self.subTest(source=source.name):
                expected = source.with_suffix(".ast").read_text(encoding="utf-8")
                result = self.run_cli(source)
                self.assertEqual(result.returncode, 0, result.stderr.decode("utf-8"))
                self.assertEqual(result.stdout.decode("utf-8"), expected)

    def test_nested_tuple_access_ast(self):
        output = self.check_valid("a.1.2.3 := 42\n")
        self.assertIn(
            "  Assignment\n"
            "    TupleIndexExpression \"3\"\n"
            "      TupleIndexExpression \"2\"\n"
            "        TupleIndexExpression \"1\"\n"
            "          Identifier \"a\"\n"
            "    IntegerLiteral \"42\"\n",
            output,
        )

    def test_syntax_errors_have_positions_and_no_partial_tree(self):
        cases = [
            ("var x :=\n", "1:9", "expected an expression"),
            ("if true then\n  print 1\n", "3:1", "expected 'end'"),
            ("x + 1\n", "1:3", "expected ':='"),
            ("var t := {}\n", "1:11", "at least one element"),
        ]
        for source, position, message in cases:
            with self.subTest(source=source):
                result = self.run_source(source)
                self.assertEqual(result.returncode, 1)
                self.assertEqual(result.stdout, b"")
                diagnostic = result.stderr.decode("utf-8")
                self.assertIn(str(self.source_path), diagnostic)
                self.assertIn("syntax error at " + position, diagnostic)
                self.assertIn(message, diagnostic)

    def test_lexical_error_is_preserved(self):
        result = self.run_source("print @\n")
        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stdout, b"")
        self.assertIn(b"lexical error at 1:7", result.stderr)

    def test_missing_file_directory_and_arguments(self):
        missing = self.run_cli(self.source_path)
        self.assertEqual(missing.returncode, 1)
        self.assertIn(b"cannot open source file", missing.stderr)

        directory = self.run_cli(self.directory.name)
        self.assertEqual(directory.returncode, 1)
        self.assertIn(b"source file", directory.stderr)

        for arguments in [(), ("first.d", "second.d")]:
            with self.subTest(arguments=arguments):
                result = self.run_cli(*arguments)
                self.assertEqual(result.returncode, 2)
                self.assertIn(b"Usage:", result.stderr)


if __name__ == "__main__":
    unittest.main(verbosity=2)
