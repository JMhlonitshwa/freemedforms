#ifndef COREIMPL_H
#define COREIMPL_H

#include <coreplugin/icore.h>

namespace Core {
    class MainWindow;
    class ActionManager;
    class ISettings;
    class IMainWindow;

namespace Internal {
    class ThemePrivate;
    class ActionManagerPrivate;
    class ContextManagerPrivate;
    class SettingsPrivate;
}
}


namespace Core {
namespace Internal {

class CoreImpl : public Core::ICore
{
    Q_OBJECT
public:
    CoreImpl(QObject *parent);
    ~CoreImpl();

    ActionManager *actionManager() const;
    ContextManager *contextManager() const;
    UniqueIDManager *uniqueIDManager() const;

    ITheme *theme() const;
    Translators *translators() const;

    ISettings *settings() const;

    IMainWindow *mainWindow() const;
    FormManager *formManager() const;


    // initialization
    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();

private:
    IMainWindow *m_MainWindow;
    ActionManagerPrivate *m_ActionManager;
    ContextManagerPrivate *m_ContextManager;
    UniqueIDManager *m_UID;
    ThemePrivate *m_Theme;
    Translators *m_Translators;
    SettingsPrivate *m_Settings;
    FormManager *m_FormManager;
};

} // namespace Internal
} // namespace Core

#endif // COREIMPL_H
