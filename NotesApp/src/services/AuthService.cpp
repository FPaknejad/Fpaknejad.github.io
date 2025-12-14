#include "AuthService.hpp"

namespace handnote::services {

AuthService::AuthService(QObject *parent)
    : QObject(parent)
{
}

void AuthService::restoreLastSession()
{
    // Placeholder: eventually load refresh token from secure storage.
    m_signedIn = false;
}

void AuthService::signIn(const QString &/*token*/)
{
    m_signedIn = true;
    emit sessionChanged(true);
}

void AuthService::signOut()
{
    m_signedIn = false;
    emit sessionChanged(false);
}

bool AuthService::isSignedIn() const
{
    return m_signedIn;
}

} // namespace handnote::services
