#pragma once

#include <QObject>
#include <QString>

namespace handnote::services {

// Placeholder authentication service so we can plug in OAuth/email later.
class AuthService : public QObject
{
    Q_OBJECT
public:
    explicit AuthService(QObject *parent = nullptr);

    void restoreLastSession();
    void signIn(const QString &token = {});
    void signOut();
    bool isSignedIn() const;

signals:
    void sessionChanged(bool signedIn);

private:
    bool m_signedIn {false};
};

} // namespace handnote::services
