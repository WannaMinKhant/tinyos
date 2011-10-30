#include <paging.h>
#include <string.h>
#include <print.h>
#include <page.h>
#include <task.h>
#include <slab.h>
#include <x86.h>
#include <int.h>
#include <mm.h>

struct task *clone_task(struct task *parent)
{
	struct task *child = alloc_task();
	if (!child)
		return NULL;
	list_add(&child->sibling, &parent->childs);
	child->parent = parent;
	return child;
}

void clone_page_table(pde_t *p, pde_t *c)
{
	pte_t *ppte = (pte_t *)VADDR(PDE_PADDR(*p));
	struct page *page;
	void *pp;
	int i;
	/* Read only? */
	if (!(*p & PDE_W))
		panic("Read only page table");
	/* mark copy-on-write flag */
	for (i = 0; i < PAGE_PTES; i++, ppte++) {
		if (!(*ppte & PTE_P))
			continue;
		page = ADDR2PG(PTE_PADDR(*ppte));
		hold_page(page);
		if (*ppte & (PTE_W|PTE_COW)) {
			if (!page->pg_cowshare) {
				*ppte &= ~PTE_W;
				*ppte |= PTE_COW;
			}
			page->pg_cowshare++;
		}
	}
	/* copy page table */
	if (!(pp = get_free_page_pa()))
		panic("Cannot get new page for cloned page table");
	memcpy((void *)VADDR(pp), (void *)VADDR(PTE_PADDR(*p)), PGSIZE);
	/* copy page dir entry */
	*c = PTE(pp, PDE_FLAGS(*p));
}

extern pde_t *kern_page_dir;
void clone_task_userspace(struct task *parent, struct task *child)
{
	pde_t *ppgdir, *cpgdir;
	int i;
	/* alloc new page dir */
	cpgdir = get_free_page();
	if (!cpgdir)
		panic("Cannot get child page dir");
	memset(cpgdir, 0x0, PGSIZE);
	child->us.pgdir = cpgdir;
	/* share the same kernel address space */
	ppgdir = parent->us.pgdir;
	memcpy(&cpgdir[KBASE_PDE], &kern_page_dir[KBASE_PDE], KBASE_PDE_SIZE);
	/* clone user space */
	for (i = 0; i < KBASE_PDE; i++) {
		if (ppgdir[i] & PDE_P)
			clone_page_table(&ppgdir[i], &cpgdir[i]);
		else
			cpgdir[i] = 0;
	}
	/* flush tlb */
	flush_tlb();
}

void clone_task_fs(struct task *parent, struct task *child)
{
	child->current_dir = parent->current_dir;
	child->root_dir = parent->root_dir;
}

void clone_task_context(struct regs *preg, struct task *child)
{
	struct regs *creg = (struct regs *)child->kstacktop - 1;
	*creg = *preg;
	creg->eax = 0;		/* Child returns 0 out of fork(). */
	child->con.esp = (unsigned int)creg;
	child->con.eip = (unsigned int)fork_child_return;
}

int fork_task(struct task *parent, struct regs *reg)
{
	struct task *child;
	child = clone_task(parent);
	if (!child)
		return -1;
	clone_task_fs(parent, child);
	clone_task_context(reg, child);
	clone_task_userspace(parent, child);
	child->state = TASK_RUNNABLE;
	return child->pid;
}

int sys_fork(void)
{
	return fork_task(ctask, ctask->reg);
}
