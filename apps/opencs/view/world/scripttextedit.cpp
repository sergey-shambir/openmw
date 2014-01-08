#include "scripttextedit.hpp"
#include "scriptcompleter.hpp"
#include <QKeyEvent>
#include <QScrollBar>

CSVWorld::ScriptTextEdit::ScriptTextEdit(QWidget *parent, const CSMWorld::Data& data)
    : QTextEdit(parent)
    , mCompleter(new QCompleter(this))
    , mCompleterModel(new QStringListModel())
    , mScriptCompleter(this, data)
{
    mCompleter->setModel (mCompleterModel);
    mCompleter->setWidget (this);
    mCompleter->setCompletionMode (QCompleter::PopupCompletion);
    mCompleter->setCaseSensitivity (Qt::CaseInsensitive);
    connect(mCompleter, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
}

void CSVWorld::ScriptTextEdit::keyPressEvent(QKeyEvent *e)
{
    if (mCompleter->popup()->isVisible())
    {
        switch (e->key())
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                e->ignore();
                return; // Pass event to code completer.
            default:
                break;
        }
    }

    QTextEdit::keyPressEvent (e);

    // Complete at key sequence Ctrl+Space.
    bool needCompletion = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space);

    if (!e->text().isEmpty())
    {
        QChar lastChar = e->text()[e->text().size() - 1];
        if (mCompleter->popup()->isVisible())
        {
            if (!lastChar.isLetterOrNumber())
            {
                mCompleter->popup()->hide();
            }
            else
            {
                QString completionPrefix = wordUnderCursor();
                mCompleter->setCompletionPrefix (completionPrefix);
                mCompleter->popup()->setCurrentIndex (mCompleter->completionModel()->index(0, 0));
            }
        }
        else if (lastChar.isLetter() && wordUnderCursor() == e->text())
        {
            // Complete when user started typing word.
            needCompletion = true;
        }
    }

    if (needCompletion)
    {
        QTextCursor tc = textCursor();
        tc.movePosition(QTextCursor::StartOfWord);

        mCompleterModel->setStringList(mScriptCompleter.codeComplete(tc));
        mCompleter->setCompletionPrefix(wordUnderCursor());

        QRect popupRect = cursorRect(tc);
        popupRect.setWidth(mCompleter->popup()->sizeHintForColumn(0)
                           + mCompleter->popup()->verticalScrollBar()->sizeHint().width());
        mCompleter->complete(popupRect);
    }
}

QString CSVWorld::ScriptTextEdit::wordUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select (QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void CSVWorld::ScriptTextEdit::insertCompletion(const QString& completion)
{
    QTextCursor tc = textCursor();
    int extra = completion.length() - mCompleter->completionPrefix().length();
    if (tc.positionInBlock() != 0)
    {
        tc.movePosition (QTextCursor::Left);
        tc.movePosition (QTextCursor::EndOfWord);
    }
    tc.insertText (completion.right(extra));
    setTextCursor (tc);
}

