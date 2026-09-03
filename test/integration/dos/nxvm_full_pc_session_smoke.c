#include "type.h"

#include "vm/composition/session/session_interface.h"
#include "test/integration/support/session_yaml.h"

C_INT main(C_INT argc, C_CHAR **argv)
{
    integration_yaml_session yaml_session;
    vm_session_reset_vector vector;
    vm_session *session;

    if (argc != 3 || integration_yaml_session_open(argv[1], argv[2],
            &yaml_session) != TYPE_STATUS_OK) return 77;
    session = yaml_session.session;
    if (
        vm_session_get_reset_vector(session, &vector) != TYPE_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        integration_yaml_session_close(&yaml_session);
        return 1;
    }
    integration_yaml_session_close(&yaml_session);
    puts("M5:T13:S8:VM-SESSION:OK");
    return 0;
}
