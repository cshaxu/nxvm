if(NOT DEFINED PROJECT_T344_CURRENT_TARGETS_FILE OR
        NOT EXISTS "${PROJECT_T344_CURRENT_TARGETS_FILE}")
    message(FATAL_ERROR "T344 current-gate target inventory is required.")
endif()
if(NOT DEFINED PROJECT_T344_CTEST_TEST_FILE OR
        NOT EXISTS "${PROJECT_T344_CTEST_TEST_FILE}")
    message(FATAL_ERROR "T344 generated CTest test file is required.")
endif()

file(STRINGS "${PROJECT_T344_CURRENT_TARGETS_FILE}" project_t344_targets)
if(project_t344_targets STREQUAL "")
    message(FATAL_ERROR "T344 current-gate target inventory is empty.")
endif()

set(project_t344_seen_targets)
foreach(project_t344_target IN LISTS project_t344_targets)
    list(FIND project_t344_seen_targets "${project_t344_target}"
        project_t344_target_index)
    if(NOT project_t344_target_index EQUAL -1)
        message(FATAL_ERROR
            "T344 current-gate target inventory is duplicated: ${project_t344_target}")
    endif()
    list(APPEND project_t344_seen_targets "${project_t344_target}")
endforeach()

file(STRINGS "${PROJECT_T344_CTEST_TEST_FILE}" project_t344_ctest_lines)
set(project_t344_registered_targets)
set(project_t344_labeled_targets)
foreach(project_t344_line IN LISTS project_t344_ctest_lines)
    if(project_t344_line MATCHES
            "^add_test\\(\\[=\\[current\\.([^]]+)\\]=\\]")
        set(project_t344_target "${CMAKE_MATCH_1}")
        list(FIND project_t344_registered_targets "${project_t344_target}"
            project_t344_registration_index)
        if(NOT project_t344_registration_index EQUAL -1)
            message(FATAL_ERROR
                "T344 CTest current registration is duplicated: ${project_t344_target}")
        endif()
        list(APPEND project_t344_registered_targets "${project_t344_target}")
    elseif(project_t344_line MATCHES "^add_test\\(\"current\\.([^\"]+)\"")
        set(project_t344_target "${CMAKE_MATCH_1}")
        list(FIND project_t344_registered_targets "${project_t344_target}"
            project_t344_registration_index)
        if(NOT project_t344_registration_index EQUAL -1)
            message(FATAL_ERROR
                "T344 CTest current registration is duplicated: ${project_t344_target}")
        endif()
        list(APPEND project_t344_registered_targets "${project_t344_target}")
    elseif(project_t344_line MATCHES
            "^set_tests_properties\\(\\[=\\[current\\.([^]]+)\\]=\\] PROPERTIES.*LABELS \"([^\"]*)\"")
        set(project_t344_target "${CMAKE_MATCH_1}")
        set(project_t344_labels "${CMAKE_MATCH_2}")
        list(FIND project_t344_labels "current-gate" project_t344_label_index)
        if(NOT project_t344_label_index EQUAL -1)
            list(FIND project_t344_labeled_targets "${project_t344_target}"
                project_t344_labeled_index)
            if(NOT project_t344_labeled_index EQUAL -1)
                message(FATAL_ERROR
                    "T344 CTest current label is duplicated: ${project_t344_target}")
            endif()
            list(APPEND project_t344_labeled_targets "${project_t344_target}")
        endif()
    elseif(project_t344_line MATCHES
            "^set_tests_properties\\(\"current\\.([^\"]+)\" PROPERTIES.*LABELS \"([^\"]*)\"")
        set(project_t344_target "${CMAKE_MATCH_1}")
        set(project_t344_labels "${CMAKE_MATCH_2}")
        list(FIND project_t344_labels "current-gate" project_t344_label_index)
        if(NOT project_t344_label_index EQUAL -1)
            list(FIND project_t344_labeled_targets "${project_t344_target}"
                project_t344_labeled_index)
            if(NOT project_t344_labeled_index EQUAL -1)
                message(FATAL_ERROR
                    "T344 CTest current label is duplicated: ${project_t344_target}")
            endif()
            list(APPEND project_t344_labeled_targets "${project_t344_target}")
        endif()
    endif()
endforeach()

foreach(project_t344_target IN LISTS project_t344_targets)
    list(FIND project_t344_registered_targets "${project_t344_target}"
        project_t344_registration_index)
    if(project_t344_registration_index EQUAL -1)
        message(FATAL_ERROR
            "T344 canonical target lacks CTest registration: ${project_t344_target}")
    endif()
    list(FIND project_t344_labeled_targets "${project_t344_target}"
        project_t344_label_index)
    if(project_t344_label_index EQUAL -1)
        message(FATAL_ERROR
            "T344 canonical target lacks current-gate label: ${project_t344_target}")
    endif()
endforeach()
foreach(project_t344_target IN LISTS project_t344_registered_targets)
    list(FIND project_t344_targets "${project_t344_target}"
        project_t344_target_index)
    if(project_t344_target_index EQUAL -1)
        message(FATAL_ERROR
            "T344 CTest current registration is outside the canonical list: ${project_t344_target}")
    endif()
endforeach()

list(LENGTH project_t344_targets project_t344_target_count)
list(LENGTH project_t344_registered_targets project_t344_registration_count)
list(LENGTH project_t344_labeled_targets project_t344_label_count)
if(NOT project_t344_target_count EQUAL project_t344_registration_count OR
        NOT project_t344_target_count EQUAL project_t344_label_count)
    message(FATAL_ERROR "T344 current-gate registration counts do not agree.")
endif()

message(STATUS
    "T344 current-gate registration passed: ${project_t344_target_count} targets.")
