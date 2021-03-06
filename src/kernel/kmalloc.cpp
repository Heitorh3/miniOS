/*
 * =====================================================================================
 *
 *       Filename:  kmalloc.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  01/31/2013 01:52:28 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include	<kernel/kmalloc.hpp>
#include	<kernel/debug.hpp>
#include	<kernel/monitor.hpp>
#include	<kernel/boot/multiboot.hpp>

uint32_t KMalloc::static_end_kmalloc_addr;

static kmalloc_handler_t kmalloc_handler=0;
static kfree_handler_t kfree_handler=0;

inline uint32_t kmalloc_basic(const uint32_t &sz, uint32_t *phys, const bool &align){
	if( align && ( end_malloc_addr()&0xFFFFF000 ) ){ // If the address is not already page-aligned
		// Align it.
		set_end_malloc_addr( end_malloc_addr()&0xFFFFF000 );
		set_end_malloc_addr( end_malloc_addr()+0x1000 );
	}
	if (phys){
		*phys=end_malloc_addr();
	}
	uint32_t tmp=end_malloc_addr();
	set_end_malloc_addr( end_malloc_addr()+sz );
	return tmp;
}

inline uint32_t kmalloc_int(const uint32_t &sz, uint32_t *phys, const bool &align){
	uint32_t ret;

	if( kmalloc_handler ){
		ret=kmalloc_handler(sz, phys, align);
	}else{
		ret=kmalloc_basic(sz, phys, align);
	}
	kdebug(4,"kmalloc_int: sz=%p ret=%p kmalloc_handler=%p\n",sz,ret,&kmalloc_handler);
	return ret;
}

inline void kfree_basic(uint32_t addr){
	kdebug(8,"kfree_basic: Not free yet! %p\n",addr);
}

inline void kfree_int(const uint32_t sz){
	kdebug(8,"free_int:\n");
	if( sz<=end_malloc_addr() ){
		kdebug(2,"kfree_int: Not free %u<=%u!\n",sz,end_malloc_addr);
		return;
	}

	kdebug(8,"free_int: kfree_handler=%p addr=%p\n",kfree_handler,sz);

	if( kfree_handler ){
		kfree_handler(sz);
	}else{
		kfree_basic(sz);
	}
	kdebug(8,"free_int: kfree_handler=%p addr=%p\n",kfree_handler,sz);
	return;
}

extern "C" {
	uint32_t kmalloc(uint32_t sz){
		return kmalloc_int(sz, 0, false);
	}

	uint32_t kmalloc_a(const uint32_t &sz ){
		return kmalloc_int(sz, 0, true);
	} 

	uint32_t kmalloc_ap(const uint32_t &sz, uint32_t *phys){
		return kmalloc_int(sz, phys, true);
	}

	uint32_t kmalloc_p(uint32_t sz, uint32_t *phys){
		return kmalloc_int(sz, phys, false);
	}

	void kmalloc_set_handler( kmalloc_handler_t new_handler){
		kmalloc_handler=new_handler;
	}

	kmalloc_handler_t kmalloc_get_handler(){
		return kmalloc_handler;
	}

	void kfree(uint32_t addr){
//		kprintf("Not free yet! %p\n",addr);
		kfree_int(addr);
	}

	void kfree_set_handler(kfree_handler_t new_handler){
		kfree_handler=new_handler;
	}

	kfree_handler_t kfree_get_handler(){
		return kfree_handler;
	}
	uint32_t end_malloc_addr(){
		return KMalloc::static_end_kmalloc_addr;
//		return static_end_kmalloc_addr;
	}
	void set_end_malloc_addr(uint32_t p){
		KMalloc::static_end_kmalloc_addr=p;
//		static_end_kmalloc_addr=p;
	}
}

