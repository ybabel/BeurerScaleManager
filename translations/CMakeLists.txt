set(QMS
    it.ts
)
list_add_prefix(QMS ${CMAKE_CURRENT_SOURCE_DIR}/)

set(BSM_QMS ${BSM_QMS} ${QMS} PARENT_SCOPE)
