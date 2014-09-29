#include <string>
#include <OTPassword.hpp>
#include <OTCallback.hpp>

#include <v8.h>

class PasswordCallback : public OTCallback
{
public:
    void runOne(const char * szDisplay, OTPassword & theOutput);
    void runTwo(const char * szDisplay, OTPassword & theOutput);

    explicit PasswordCallback(std::string passphrase);
    ~PasswordCallback();
private:   
    std::string m_passphrase;
};
