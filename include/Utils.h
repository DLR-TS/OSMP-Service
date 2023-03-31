/**
@authors German Aerospace Center: Björn Bahn
*/

#ifndef UTILS_H
#define UTILS_H

#include "OSIMessages.h"

eOSIMessage getMessageType(const std::string& messageType);
bool matchingNames(const std::string& name1, const std::string& name2);

#endif // !UTILS_H
