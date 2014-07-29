#include <string>
#include <iostream>
#include <OTPassword.hpp>
#include <passwordCallback.hpp>

#include <v8.h>


PasswordCallback::PasswordCallback(std::string passphrase) : m_passphrase(passphrase) {
}

PasswordCallback::~PasswordCallback() {
}


void PasswordCallback::runOne(const char * szDisplay, OTPassword & theOutput)
{
    theOutput.setPassword(m_passphrase.c_str(), m_passphrase.size());
}

void PasswordCallback::runTwo(const char * szDisplay, OTPassword & theOutput)
{
    theOutput.setPassword(m_passphrase.c_str(), m_passphrase.length());
}
