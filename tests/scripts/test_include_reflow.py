import os
import sys
import unittest

# Make the .scripts directory importable
SCRIPT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '.scripts'))
if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)

import include_reflow

class TestIncludeReflow(unittest.TestCase):
    def test_ordering_platform_stl_qt_modules_sdk_thirdparty_system_project(self):
        input_contents = (
            '// header comment\n'
            '#include "local.h"\n'
            '#include <QtCore/QObject>\n'
            '#include <algorithm>\n'
            '#include <windows.h>\n'
            '#include <Common/SomeModule.h>\n'
            '#include <SDK/Api.h>\n'
            '#include <boost/boost.h>\n'
            '#include <sys/types.h>\n'
        )

        expected = (
            '// header comment\n'
            '\n'
            '// Platform\n'
            '#include <windows.h>\n'
            '\n'
            '// STL\n'
            '#include <algorithm>\n'
            '\n'
            '// Qt\n'
            '#include <Common/QtHeadersBegin.h>\n'
            '#include <QtCore/QObject>\n'
            '#include <Common/QtHeadersEnd.h>\n'
            '\n'
            '// Modules\n'
            '#include <Common/SomeModule.h>\n'
            '\n'
            '// SDK\n'
            '#include <SDK/Api.h>\n'
            '\n'
            '// ThirdParty\n'
            '#include <boost/boost.h>\n'
            '\n'
            '// System\n'
            '#include <sys/types.h>\n'
            '\n'
            '// Project\n'
            '#include "local.h"\n'
        )

        new_contents, changed = include_reflow.reflow_file(input_contents)
        self.assertTrue(changed, "Expected changes to be reported by reflow_file")
        # Normalize trailing newline as the script appends depending on input
        self.assertEqual(new_contents, expected)

    def test_qt_wrapper_is_added_for_qt_includes(self):
        # header-style file with header guard before includes should be handled
        input_contents = (
            '#ifndef TEST_H\n'
            '#define TEST_H\n'
            '\n'
            '#include <QtCore/QProcess>\n'
            '#include <QtNetwork/QNetworkAccessManager>\n'
            '\n'
            'class Foo;\n'
            '#endif\n'
        )
        expected = (
            '#ifndef TEST_H\n'
            '#define TEST_H\n'
            '\n'
            '// Qt\n'
            '#include <Common/QtHeadersBegin.h>\n'
            '#include <QtCore/QProcess>\n'
            '#include <QtNetwork/QNetworkAccessManager>\n'
            '#include <Common/QtHeadersEnd.h>\n'
            '\n'
            'class Foo;\n'
            '#endif\n'
        )
        new_contents, changed = include_reflow.reflow_file(input_contents)
        self.assertTrue(changed)
        self.assertEqual(new_contents, expected)

        input_contents = (
            '#include <QtCore/QProcess>\n'
            '#include <QtCore/QThread>\n'
            '#include <QtNetwork/QNetworkAccessManager>\n'
            '#include "local.h"\n'
        )
        expected = (
            '// Qt\n'
            '#include <Common/QtHeadersBegin.h>\n'
            '#include <QtCore/QProcess>\n'
            '#include <QtCore/QThread>\n'
            '#include <QtNetwork/QNetworkAccessManager>\n'
            '#include <Common/QtHeadersEnd.h>\n'
            '\n'
            '// Project\n'
            '#include "local.h"\n'
        )
        new_contents, changed = include_reflow.reflow_file(input_contents)
        self.assertTrue(changed)
        self.assertEqual(new_contents, expected)

    def test_preserve_top_block_comment_and_qt_wrapper(self):
        input_contents = (
            '/* @file Реализация интерфейсов для работы с БД. */\n'
            '\n'
            '#include <QtCore/QScopedPointer>\n'
            '#include <QtCore/QFile>\n'
            '#include <QtCore/QRegExp>\n'
            '#include <QtCore/QStringList>\n'
            '#include <QtCore/QMutexLocker>\n'
            '\n'
            '#include "local.h"\n'
        )
        expected = (
            '/* @file Реализация интерфейсов для работы с БД. */\n'
            '\n'
            '// Qt\n'
            '#include <Common/QtHeadersBegin.h>\n'
            '#include <QtCore/QFile>\n'
            '#include <QtCore/QMutexLocker>\n'
            '#include <QtCore/QRegExp>\n'
            '#include <QtCore/QScopedPointer>\n'
            '#include <QtCore/QStringList>\n'
            '#include <Common/QtHeadersEnd.h>\n'
            '\n'
            '// Project\n'
            '#include "local.h"\n'
        )
        new_contents, changed = include_reflow.reflow_file(input_contents)
        self.assertTrue(changed)
        self.assertEqual(new_contents, expected)

    def test_idempotent_on_already_formatted_content(self):
        input_contents = (
            '// Qt\n'
            '#include <Common/QtHeadersBegin.h>\n'
            '#include <QtCore/QProcess>\n'
            '#include <Common/QtHeadersEnd.h>\n'
        )
        new_contents, changed = include_reflow.reflow_file(input_contents)
        self.assertFalse(changed)
        self.assertEqual(new_contents, input_contents)


if __name__ == '__main__':
    unittest.main()
