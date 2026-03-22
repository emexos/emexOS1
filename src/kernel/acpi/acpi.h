#pragma once

#include <types.h>
#include <kernel/modules/limine.h>

#define SCI_EN (1 << 0)
#define SLP_EN (1 << 13)

struct acpi_header {
    char signature[4];
    u32 length;
    u8 revision;
    u8 checksum;
    char oem_id[6];
    char oem_table_id[8];
    u32 oem_revision;
    u32 creator_id;
    u32 creator_revision;
} __attribute__((packed));

struct rsdp_descriptor {
    char signature[8];
    u8 checksum;
    char oem_id[6];
    u8 revision;
    u32 rsdt_address;
    u32 length;
    u64 xsdt_address;
    u8 ext_checksum;
    u8 reserved[3];
} __attribute__((packed));

struct acpi_gas {
    u8 address_space_id; // 0 = MMIO, 1 = I/O
    u8 register_bit_width;
    u8 register_bit_offset;
    u8 access_size;
    u64 address;
} __attribute__((packed));

struct fadt_descriptor {
    struct acpi_header header;
    u32 firmware_ctrl;
    u32 dsdt;
    u8 reserved;
    u8 preferred_pm_profile;
    u16 sci_int;
    u32 smi_cmd;
    u8 acpi_enable;
    u8 acpi_disable;
    u8 s4bios_req;
    u8 pstate_cnt;
    u32 pm1a_evt_blk;
    u32 pm1b_evt_blk;
    u32 pm1a_cnt_blk;
    u32 pm1b_cnt_blk;
    u32 pm2_cnt_blk;
    u32 pm_tmr_blk;
    u32 gpe0_blk;
    u32 gpe1_blk;
    u8 pm1_evt_len;
    u8 pm1_cnt_len;
    u8 pm2_cnt_len;
    u8 pm_tmr_len;
    u8 gpe0_blk_len;
    u8 gpe1_blk_len;
    u8 gpe1_base;
    u8 cst_cnt;
    u16 p_lvl2_lat;
    u16 p_lvl3_lat;
    u16 flush_size;
    u16 flush_stride;
    u8 duty_offset;
    u8 duty_width;
    u8 day_alrm;
    u8 mon_alrm;
    u8 century;
    u16 iapc_boot_arch;
    u8 reserved2;
    u32 flags;
    struct acpi_gas reset_reg;
    u8 reset_value;
    u8 reserved3[3];
    u64 x_firmware_ctrl;
    u64 x_dsdt;
    struct acpi_gas x_pm1a_evt_blk;
    struct acpi_gas x_pm1b_evt_blk;
    struct acpi_gas x_pm1a_cnt_blk;
    struct acpi_gas x_pm1b_cnt_blk;
    struct acpi_gas x_pm2_cnt_blk;
    struct acpi_gas x_pm_tmr_blk;
    struct acpi_gas x_gpe0_blk;
    struct acpi_gas x_gpe1_blk;
} __attribute__((packed));

#define AML_NS_MAX 4096
#define AML_NAME_MAX  128
#define AML_DEPTH_MAX 12

typedef struct {
    u64 args[7];
    u64 locals[8];
    char scope[AML_NAME_MAX];
    int depth;
} aml_ctx_t;

typedef enum {
    AML_NONE = 0, AML_INT, AML_METHOD, AML_REGION, AML_FIELD
} aml_type_t;

typedef struct {
    char       path[AML_NAME_MAX];
    aml_type_t type;
    union {
        u64 ival;
        struct { u8 *body; u32 blen; u8 argc; } method;
        struct {
            u8  space;                  // 0=Mem 1=IO
            int      dyn;                    // base is a runtime field ref
            u64 base;
            char     base_fld[AML_NAME_MAX]; // if dyn: absolute path of base field
            u32 len;
        } region;
        struct {
            char     rgn[AML_NAME_MAX];      // absolute path of owning region
            u32 bit_off;
            u32 bit_wid;
        } field;
    };
} aml_obj_t;

// Result types
#define RET_VAL  0  // normal integer result
#define RET_VOID 1  // void (no value)
#define RET_RET  2  // return statement hit

typedef struct { u64 v; int t; } aml_val_t;
#define V(x)  ((aml_val_t){.v=(x),.t=RET_VAL})
#define VOID  ((aml_val_t){.t=RET_VOID})
#define RET(x)((aml_val_t){.v=(x),.t=RET_RET})

#define PM1_CNT_BM_RLD       (1u << 1)
#define PM1_CNT_SLP_TYP_MASK (0x7u << 10)
#define PM1_CNT_SLP_EN       (1u << 13)
#define PM1_CNT_WRITE_MASK   (PM1_CNT_SLP_TYP_MASK | PM1_CNT_SLP_EN | PM1_CNT_BM_RLD)

void* find_acpi_table(const char* sig);
void init_acpi(void);
void shutdown(void);
void reboot(void);