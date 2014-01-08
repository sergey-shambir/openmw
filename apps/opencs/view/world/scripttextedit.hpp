#ifndef CSV_WORLD_SCRIPTTEXTEDIT_H
#define CSV_WORLD_SCRIPTTEXTEDIT_H

#include "scriptcompleter.hpp"
#include <QTextEdit>
#include <QCompleter>
#include <QStringListModel>

namespace CSMWorld
{
    class Data;
}

namespace CSVWorld
{
    class ScriptTextEdit : public QTextEdit
    {
            Q_OBJECT
        public:
            ScriptTextEdit(QWidget *parent, const CSMWorld::Data& data);

        protected:
            void keyPressEvent(QKeyEvent *e);
            QString wordUnderCursor() const;

        private slots:
            void insertCompletion(const QString& completion);

        private:
            QCompleter *mCompleter;
            QStringListModel *mCompleterModel;
            ScriptCompleter mScriptCompleter;
    };
}

#endif
