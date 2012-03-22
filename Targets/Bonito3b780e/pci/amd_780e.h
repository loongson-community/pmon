#ifndef AMD_780E_H
#define AMD_780E_H

#include "rs780.h"

void rs780_early_setup(void);
void sb700_early_setup(void);

int optimize_link_incoherent_ht(void);
void soft_reset(void);

void rs780_before_pci_fixup(void);
void sb700_before_pci_fixup(void);


void rs780_after_pci_fixup(void);
void sb700_after_pci_fixup(void);

#endif
