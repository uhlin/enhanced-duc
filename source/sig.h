#ifndef GUARD_SIG_H
#define GUARD_SIG_H

#include "ducdef.h"

__DUC_BEGIN_DECLS
void	block_signals(void);
void	program_clean_up(void);
int	sighand_init(void);
__DUC_END_DECLS

#endif
