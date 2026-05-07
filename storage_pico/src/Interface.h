#include "../../../shared/Types.h"

class Interface
{
public:

bool hasCommand();
CMDType getCMD();
int getPosition();

void sendResponse(cmdType, ResponseType);
void sendEvent(cmdTYpe, EventType, EventReason);

private:
int lastPosition;

}