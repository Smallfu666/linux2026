#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define container_of(ptr, type, member) \
    ({ \
        const typeof(((type *)0)->member) *__mptr = (ptr); \
        (type *)((char *)__mptr - offsetof(type, member)); \
    })

struct kobject_like {
    const char *name;
    unsigned int refcount;
};

struct device_like {
    uint32_t device_number;
    struct kobject_like kobj;
    const char *driver_name;
};

struct pci_dev_like {
    uint16_t vendor_id;
    uint16_t device_id;
    struct device_like dev;
    uint8_t bus;
    uint8_t slot;
};

static void print_layout(void)
{
    printf("struct kobject_like: sizeof=%zu alignof=%zu\n",
           sizeof(struct kobject_like), _Alignof(struct kobject_like));
    printf("struct device_like:  sizeof=%zu alignof=%zu\n",
           sizeof(struct device_like), _Alignof(struct device_like));
    printf("struct pci_dev_like: sizeof=%zu alignof=%zu\n",
           sizeof(struct pci_dev_like), _Alignof(struct pci_dev_like));
    printf("offsetof(device_like, kobj)=%zu\n", offsetof(struct device_like, kobj));
    printf("offsetof(pci_dev_like, dev)=%zu\n", offsetof(struct pci_dev_like, dev));
    printf("offsetof(pci_dev_like, dev.kobj)=%zu\n",
           offsetof(struct pci_dev_like, dev) + offsetof(struct device_like, kobj));
}

int main(void)
{
    struct pci_dev_like pci = {
        .vendor_id = 0x8086,
        .device_id = 0x100e,
        .dev = {
            .device_number = 17,
            .kobj = {
                .name = "0000:00:03.0",
                .refcount = 2,
            },
            .driver_name = "e1000-demo",
        },
        .bus = 0,
        .slot = 3,
    };
    struct kobject_like *kobj = &pci.dev.kobj;
    struct device_like *dev = container_of(kobj, struct device_like, kobj);
    struct pci_dev_like *pdev = container_of(dev, struct pci_dev_like, dev);

    print_layout();
    puts("");
    printf("pci_dev_like @ %p\n", (void *)&pci);
    printf("device_like  @ %p\n", (void *)&pci.dev);
    printf("kobject_like @ %p\n", (void *)&pci.dev.kobj);
    printf("recovered device_like  @ %p\n", (void *)dev);
    printf("recovered pci_dev_like @ %p\n", (void *)pdev);
    puts("");
    printf("vendor_id=%#x device_id=%#x driver=%s\n",
           pci.vendor_id, pci.device_id, pci.dev.driver_name);
    printf("kobject name=%s refcount=%u\n", kobj->name, kobj->refcount);
    printf("recovery_ok=%s\n", (dev == &pci.dev && pdev == &pci) ? "yes" : "no");
    return 0;
}
