#include <stdio.h>
#include "privilege.h"

int main()
{
    printf("[*] Trying to enable SeDebugPrivilege (allows full access to other processes)...\n");
    EnablePrivilege(L"SeDebugPrivilege");

    printf("\n[*] Trying to enable SeLoadDriverPrivilege (allows loading kernel drivers)...\n");
    EnablePrivilege(L"SeLoadDriverPrivilege");

    return 0;
}
