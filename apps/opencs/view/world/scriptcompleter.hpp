#ifndef CSV_WORLD_SCRIPTCOMPLETER_H
#define CSV_WORLD_SCRIPTCOMPLETER_H

#include <QObject>
#include <QString>
#include <vector>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QTextCursor;
QT_END_NAMESPACE

namespace CSMWorld
{
    class Data;
}

namespace CSVWorld
{
    class ScriptCompleter
    {
        public:
            ScriptCompleter(QTextEdit *editor, const CSMWorld::Data& data);

            QStringList codeComplete(const QTextCursor &cursor);
            void invalidateIds();

        private:
            class Data;
            Data *d;
    };
}

#endif
