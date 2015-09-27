#include <string>
#include <OTPassword.hpp>
#include <OTCallback.hpp>

#include <v8.h>

using namespace opentxs;

class PasswordCallback : public OTCallback
{
public:
    void runOne(const char * szDisplay, OTPassword & theOutput) const;
    void runTwo(const char * szDisplay, OTPassword & theOutput) const;

    explicit PasswordCallback(std::string passphrase);
    ~PasswordCallback();
private:   
    std::string m_passphrase;
};
