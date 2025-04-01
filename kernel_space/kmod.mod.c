#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_MITIGATION_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const char ____versions[]
__used __section("__versions") =
	"\x20\x00\x00\x00\x9a\xfc\x00\xc3"
	"genl_register_family\0\0\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x14\x00\x00\x00\x63\x92\x7d\xcc"
	"kobject_put\0"
	"\x20\x00\x00\x00\xf5\xe1\x3d\xfd"
	"sysfs_remove_file_ns\0\0\0\0"
	"\x20\x00\x00\x00\x31\xb5\x22\x0c"
	"genl_unregister_family\0\0"
	"\x14\x00\x00\x00\x24\xc9\x05\x52"
	"__alloc_skb\0"
	"\x14\x00\x00\x00\xee\xd1\x81\x94"
	"genlmsg_put\0"
	"\x10\x00\x00\x00\x9c\x53\x4d\x75"
	"strlen\0\0"
	"\x10\x00\x00\x00\xe5\x6c\xd8\xe4"
	"nla_put\0"
	"\x14\x00\x00\x00\x55\x70\xa8\x6a"
	"init_net\0\0\0\0"
	"\x24\x00\x00\x00\x80\x49\xf6\xef"
	"netlink_broadcast_filtered\0\0"
	"\x14\x00\x00\x00\x81\x32\x45\x33"
	"skb_trim\0\0\0\0"
	"\x1c\x00\x00\x00\x81\xa4\x5c\xe8"
	"sk_skb_reason_drop\0\0"
	"\x18\x00\x00\x00\x32\x05\x7e\xbc"
	"netlink_unicast\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x10\x00\x00\x00\x7e\x3a\x2c\x12"
	"_printk\0"
	"\x20\x00\x00\x00\x5d\x95\x77\xb4"
	"kobject_create_and_add\0\0"
	"\x20\x00\x00\x00\x74\x1d\x46\x60"
	"sysfs_create_file_ns\0\0\0\0"
	"\x18\x00\x00\x00\xde\x9f\x8a\x25"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "AEE7EF7449C94E3FDFAD8E9");
