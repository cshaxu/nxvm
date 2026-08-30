#include "type.h"

#include "core/machine/dma.h"
#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "vm/profile/default_profile/pc_at_profile_private.h"
#include "vm/profile/model40/model40_private.h"
#include "vm/profile/xt/xt_5160_268.h"

typedef C_INT vm_timing_qualification_configure(
    core_machine_config *out_configuration);

typedef struct vm_timing_qualification_case {
    const C_CHAR *name;
    vm_timing_qualification_configure *configure;
} vm_timing_qualification_case;

static C_INT vm_timing_qualification_default_configure(
    core_machine_config *out_configuration)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_default_pc_at_descriptor_get();
    vm_profile_default_pc_at_cpu_contract contract;
    core_machine_controller_timing_rules rules;

    return out_configuration == STD_NULL || profile == STD_NULL ||
        !vm_profile_default_pc_at_cpu_contract_select(profile, profile->cpu_profile,
            profile->fpu_profile, &contract) ||
        !vm_profile_default_pc_at_core_config_materialize(profile, &contract,
            out_configuration, &rules);
}

static C_INT vm_timing_qualification_model_339_configure(
    core_machine_config *out_configuration)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_ibm_5170_model_339_descriptor_get();
    vm_profile_default_pc_at_cpu_contract contract;
    core_machine_controller_timing_rules rules;

    return out_configuration == STD_NULL || profile == STD_NULL ||
        !vm_profile_default_pc_at_cpu_contract_select(profile, profile->cpu_profile,
            profile->fpu_profile, &contract) ||
        !vm_profile_default_pc_at_core_config_materialize(profile, &contract,
            out_configuration, &rules);
}

static C_INT vm_timing_qualification_model_40_configure(
    core_machine_config *out_configuration)
{
    if (out_configuration == STD_NULL) return 1;
    vm_profile_model40_core_config_initialize(out_configuration);
    return 0;
}

static C_INT vm_timing_qualification_xt_configure(
    core_machine_config *out_configuration)
{
    vm_profile_xt_5160_268_resolved_profile profile;

    if (out_configuration == STD_NULL ||
        vm_profile_xt_5160_268_resolve(&profile, TYPE_FALSE) != TYPE_STATUS_OK) {
        return 1;
    }
    *out_configuration = profile.resolved.values.core.configuration;
    return 0;
}

static C_INT vm_timing_qualification_assert_case(
    const vm_timing_qualification_case *test_case)
{
    core_machine_config configuration = {0};
    core_machine_dma_wiring wiring = {0};
    core_machine_dma_request_binding request = {0};
    core_machine_time_observation observation;
    core_machine_plan *plan = STD_NULL;
    core_machine *machine = STD_NULL;
    type_status status;
    C_INT failed = 0;

    if (test_case == STD_NULL || test_case->configure == STD_NULL ||
        test_case->configure(&configuration) ||
        configuration.clock_plan.dma.numerator == 0u ||
        configuration.clock_plan.dma.denominator == 0u) return 1;
    wiring.fdc_channel = 2u;
    wiring.controller_count = configuration.dma_controller_count;
    wiring.cascade_channel = configuration.dma_controller_count == 1u ? 0u :
        CORE_MACHINE_DMA_CASCADE_CHANNEL;
    status = core_machine_plan_create(&configuration, &plan);
    if (status != TYPE_STATUS_OK) {
        STD_PRINTF("%s: plan create status %d\n", test_case->name, status);
        return 1;
    }
    status = core_machine_create_from_plan(plan, &machine);
    if (status != TYPE_STATUS_OK) {
        STD_PRINTF("%s: core create status %d\n", test_case->name, status);
        failed = 1;
    }
    if (!failed) {
        status = core_machine_configure_dma(machine, &wiring, &request);
        if (status != TYPE_STATUS_OK || request.core_token == 0u) {
            STD_PRINTF("%s: DMA configuration status %d\n", test_case->name, status);
            failed = 1;
        }
    }
    if (!failed) {
        status = core_machine_freeze_execution_providers(machine);
        if (status != TYPE_STATUS_OK) {
            STD_PRINTF("%s: provider freeze status %d\n", test_case->name, status);
            failed = 1;
        }
    }
    if (!failed) {
        status = core_machine_reset(machine);
        if (status != TYPE_STATUS_OK) {
            STD_PRINTF("%s: machine reset status %d\n", test_case->name, status);
            failed = 1;
        }
    }
    if (!failed) {
        core_machine_port_write(&machine->executor_port, 0x000au, 0x02u);
        core_machine_dma_request_assert(&machine->shared_dma_primary,
            &machine->shared_dma_secondary, &request);
        status = core_machine_capture_time_observation(machine, &observation);
        if (status != TYPE_STATUS_OK || !observation.next_deadline_valid ||
            observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_DEADLINE) {
            STD_PRINTF("%s: observation status %d deadline %u disposition %d\n",
                test_case->name, status, observation.next_deadline_valid,
                observation.progress_disposition);
            failed = 1;
        }
    }
    core_machine_destroy(machine);
    core_machine_plan_destroy(plan);
    return failed;
}

int main(void)
{
    static const vm_timing_qualification_case cases[] = {
        {"default-pc-at", vm_timing_qualification_default_configure},
        {"ibm-5170-model-339", vm_timing_qualification_model_339_configure},
        {"compaq-deskpro-386-model-40", vm_timing_qualification_model_40_configure},
        {"ibm-5160-model-268", vm_timing_qualification_xt_configure}
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        if (vm_timing_qualification_assert_case(&cases[index])) {
            STD_PRINTF("timing qualification failed: %s\n", cases[index].name);
            return 1;
        }
    }
    STD_PRINTF("M5:T508:S4:FOUR-PROFILE-DMA-DEADLINE:OK\n");
    return 0;
}
