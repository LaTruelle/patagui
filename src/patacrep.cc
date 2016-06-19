#include "patacrep.hh"

#include <QDebug>

Patacrep::Patacrep(QObject *parent) : QObject(parent)
{
    // Setup Python interpreter
    PythonQt::init(PythonQt::RedirectStdOut);
    pythonModule = PythonQt::self()->getMainModule();
    // Redirect Std Out and Std Err to qDebug output through slot for easier
    // debug.
    connect(PythonQt::self(), SIGNAL(pythonStdOut(QString)),
            SLOT(stdOut(QString)));
    connect(PythonQt::self(), SIGNAL(pythonStdErr(QString)),
            SLOT(stdErr(QString)));
    connect(this, SIGNAL(message(QString, int)), SLOT(debugOutput(QString)));
    // Import Python file containing all necessary functions and imports
    pythonModule.evalFile(":/python_scripts/songbook.py");
    buildingSongbook = false;
}

Patacrep::~Patacrep()
{
}

void Patacrep::stdOut(QString string)
{
    // Hack to simplify output
    if (!string.simplified().isEmpty()) {
        emit message("PyOut: " + string.simplified(), 0);
    }
}

void Patacrep::stdErr(QString string)
{
    // Hack to simplify output
    if (!string.simplified().isEmpty()) {
        emit message("PyErr: " + string.simplified(), 0);
        // qWarning() << string.simplified().toUtf8().constData();
    }
}

void Patacrep::setWorkingDirectory(const QString &dir)
{
    pythonModule.evalScript("os.chdir('" + dir + "')");
}

bool Patacrep::testPython()
{
    // FIXME Test python
    return true;
}

bool Patacrep::testPatacrep()
{
    // FIXME Test patacrep
    return true;
}

Songbook *Patacrep::getSongbook() const
{
    return songbook;
}

void Patacrep::setSongbook(Songbook *value)
{
    songbook = value;
}

QStringList Patacrep::getDatadirs() const
{
    return datadirs;
}

void Patacrep::buildSongbook()
{
    buildingSongbook = true;
    emit(aboutToStart());
    if (!songbook->filename().isEmpty()) {
        // Expose Songbook to python
        pythonModule.addObject("songbook", songbook);
        pythonModule.addObject("CPPprocess", this);
        pythonModule.evalScript("setupSongbook(songbook.filename)");
        pythonModule.evalScript(
            "build(['clean', 'tex', 'pdf', 'sbx', 'pdf', 'clean'])");
        //        pythonModule.removeVariable("songbook");
        emit(message("Finished Execution", 0));
        emit(finished());
    } else {
        emit(message("Error: no songbook loaded", 0));
        emit(finished());
    }
}

void Patacrep::debugOutput(QString string)
{
    qDebug() << string;
}

bool Patacrep::getBuildState() const
{
    return buildingSongbook;
}

void Patacrep::setDatadirs(const QStringList &datadirs)
{
    Patacrep::datadirs.append(datadirs);
}

void Patacrep::addDatadir(const QString &datadir)
{
    datadirs.append(datadir);
}

void Patacrep::stopBuilding()
{
    buildingSongbook = false;
}
