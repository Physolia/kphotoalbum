#include "RunDialog.h"
#include <KDialog>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <MainWindow/Window.h>
#include <klocale.h>
#include <krun.h>
#include <kshell.h>

MainWindow::RunDialog::RunDialog( QWidget* parent, QStringList fileList )
    : KDialog( parent ), _fileList(fileList)
{
    QWidget* top = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout( top );
    setMainWidget(top);

    QString txt = i18n("<p>Enter your command to run below:</p>"
                       "<p><i>%all will be replaced with a file list</i></p>");
    QLabel* label = new QLabel(txt);
    layout->addWidget(label);

    _cmd = new QLineEdit();
    layout->addWidget(_cmd);
    txt = i18n("<p>Enter the command you want to run on your image file(s).  "
               "KPhotoAlbum will run your command and replace any '%all' tokens "
               "with a list of your files.  For example, if you entered:</p>"
               "<ul><li>cp %all /tmp</li></ul>"
               "<p>Then the files you selected would be copied to the /tmp "
               "directory</p>"
               "<p>You can also use %each to have a command be run once per "
               "file.</p>");
    _cmd->setWhatsThis(txt);
    label->setWhatsThis(txt);

    connect( this, SIGNAL( okClicked() ), this, SLOT( slotMarkGo() ) );
}

void MainWindow::RunDialog::slotMarkGo( )
{
    QString cmdString = _cmd->text();
    QRegExp replaceall = QRegExp(i18n("%all"));
    QRegExp replaceeach = QRegExp(i18n("%each"));

    // Replace the %all argument first
    cmdString.replace(replaceall, KShell::joinArgs(_fileList));

    if (cmdString.contains(replaceeach)) {
        // cmdString should be run multiple times, once per "each"
        QString cmdOnce;
        for( QStringList::Iterator it = _fileList.begin(); it != _fileList.end(); ++it ) {
            cmdOnce = cmdString;
            cmdOnce.replace(replaceeach, *it);
            KRun::runCommand(cmdOnce, MainWindow::Window::theMainWindow());
        }
    } else {
        KRun::runCommand(cmdString, MainWindow::Window::theMainWindow());
    }
}


void MainWindow::RunDialog::show()
{
    KDialog::show();
}
