/*
 * dwarf.c
 *
 *  Created on: 10.08.2014
 *      Author: Daniel
 */
#include <inc/types.hh>
#include <kernel/Kernel.hh>
#include "SCLConfig.hh"

#if DWARF_DEBUG

extern Kernel* theOS;
extern void* _debug_frame;
extern void* _debug_frame_end;

/* pointer to the dwarf debug.frames section*/
static void* frame_info;

typedef struct {
    void*  location;
    unint4 cfa_offset;      /* call frame offset at this location */
    unint4 stored_regs[16]; /* stored register offsets on the call frame addresses at this location*/
} cf_table_entry;

typedef struct {
    unint4 length;
    unint1 version;
    char*  augmentation;
    int4   code_alignment;
    int4   data_alignment;
    unint1 return_reg;
    void*  initial_regs[16]; /* initial register config */
} CIE;

typedef struct {
    unint4 length;
    unint4 CIE_ptr;
    unint4 location;
    unint4 addr_range;
} FDE_Header;

typedef struct {
    unint4 address;
    unint4 length;
    char* signature;
} DebugStrTableT;

/* str table is auto generated and will overload this symbol at link time!*/
DebugStrTableT __attribute__((weak)) debug_strtable[1] = {{0,0xffffffff,"Unknown"}};
unsigned int  __attribute__((weak)) debug_strtable_entries = 0;


static CIE cie;
void* dwarf_parseCIE(void* address, CIE* cie);



int4 decodeLEB128(char value) {
    int4 result = 0;
    result = value & 0x7f;

    /* sign extend */
    if (result & 0x40)
        result = (-128) + result;

    return (result);
}

unint4 decodeULEB128(char value) {
    char result = 0;
    result = value & 0x7f;

    return (result);
}


void dwarf_init() {

    LOG(ARCH,INFO,"DWARF init debug.frames at 0x%x",&_debug_frame);
    LOG(ARCH,INFO,"debug_strtable_entries = %d",debug_strtable_entries);
    frame_info = &_debug_frame;
    if (frame_info) {

        void* addr =dwarf_parseCIE(frame_info,&cie);
        if (addr == 0) {
            LOG(ARCH,WARN,"DWARF: debug.frames section corrupt");
            frame_info = 0;

        } else
            LOG(ARCH,INFO,"DWARF valid debug.frames found at 0x%x",frame_info);
    }
}

void* dwarf_parseCIE(void* address, CIE* cie) {

    char*   stream_ptr;
    unint4* addr = (unint4*) address;
    cie->length = *addr;
    addr++;
    unint4 entry_id = *addr;
    addr++;
    if (entry_id != 0xffffffff)
        return (0);

    stream_ptr = (char*) addr;
    cie->version = *stream_ptr;
    /* check version*/
    if (cie->version != 1) {
        LOG(ARCH,WARN,"DWARF: Unknown CIE version %d",cie->version);
        return (0);
    }

    stream_ptr++;
    /* extract augmentation */
    cie->augmentation = 0;
    while (*stream_ptr) stream_ptr++;
    stream_ptr++;
    cie->code_alignment = decodeLEB128(*stream_ptr);
    stream_ptr++;
    cie->data_alignment = decodeLEB128(*stream_ptr);
    stream_ptr++;
    cie->return_reg = *stream_ptr;
    /* following the scratch register initialization program*/
    // TODO: parse CIE init register scratch program



    return ((void*) (((size_t) address + cie->length + 4)));
}


FDE_Header* dwarf_getFDE(void* address, CIE* cie) {
    if (frame_info == 0)
        return (0);

    void* addr = frame_info;
    /* first must be a CIE header */
    addr = dwarf_parseCIE(addr,cie);
    if (addr == 0) {
        LOG(ARCH,WARN,"DWARF: expected CIE header");
        /* ERROR wrong entry type */
        return (0);
    }

    FDE_Header* fde_head = (FDE_Header*) addr;

    while ((void*) fde_head < &_debug_frame_end &&
           ! ( fde_head->location <= (unint4) address &&
              (fde_head->location + fde_head->addr_range) > (unint4)address)) {

        /* iterate through the fde list until we get our entry or another CIE follows*/
        while ((void*) fde_head < &_debug_frame_end && fde_head->CIE_ptr != 0xffffffff &&
               ! ( fde_head->location <= (unint4) address &&
                  (fde_head->location + fde_head->addr_range) > (unint4)address)) {
            fde_head = (FDE_Header*) (((unint4) fde_head) + fde_head->length + 4);
        }

        if ((void*) fde_head < &_debug_frame_end && fde_head->location <= (unint4) address &&
            (fde_head->location + fde_head->addr_range) > (unint4)address)
            return (fde_head);

        if ((void*) fde_head >= &_debug_frame_end)
              return (0);

        /* should be another CIE following */
        addr = dwarf_parseCIE(fde_head,cie);
        if (addr == 0) {
            LOG(ARCH,WARN,"DWARF: expected valid CIE header");
            /* ERROR wrong entry type */
            return (0);
        }

        fde_head = (FDE_Header*) addr;
    }

    if ((void*) fde_head >= &_debug_frame_end)
        return (0);

    /* fde here! */
    return (fde_head);

}


static cf_table_entry entry;



bool dwarf_interpret_cfa(cf_table_entry* entry, unint1*& stream, CIE* cie) {

   // LOG(ARCH,INFO,"dwarf_interpret_cfa %x",*stream);
    unint1 highBits = *stream >> 6;
    int lowBits   = *stream & 0x3f;
    stream++;

    switch (highBits) {
    case 1 : {
        /* DW_CFA_advance_loc */
        LOG(ARCH,DEBUG,"DW_CFA_advance_loc %d",cie->code_alignment * lowBits);
        entry->location = (void*) ((size_t) entry->location + cie->code_alignment * lowBits);
        return (true);
    }
    case 2 :  {
        /*DW_CFA_offset */
        char operand = decodeULEB128(*stream);
        stream++;
        LOG(ARCH,DEBUG,"DW_CFA_offset r%d %d",lowBits, operand * cie->data_alignment);
        entry->stored_regs[lowBits] = operand * cie->data_alignment;
        return (true);
    }
    case 3 : {
        /* DW_CFA_restore*/
        entry->stored_regs[lowBits] =   (size_t) cie->initial_regs[lowBits];
        return (true);
    }
    case 0 : {
        switch (lowBits) {
            case 0: { /*NOP */ return (true); }
            case 2: {
                unsigned char operand = *stream;
                stream++;
                entry->location = (void*) ((size_t) entry->location + operand);
                return (true);
            }
            case 0xe: {
                /* DW_CFA_def_cfa_offset */
                char operand = *stream;
                stream++;
                LOG(ARCH,DEBUG,"DW_CFA_def_cfa_offset %d",decodeULEB128(operand));
                int newoffset = decodeULEB128(operand);
                entry->cfa_offset = newoffset;
                return (true);
            }
            case 0xa: {
                /* save current state*/
                /* TODO */
            }
            case 0xb: {
                /* restore state..*/
                /* TODO*/
            }

            default: {
                //LOG(ARCH,WARN,"Unknown DW_CFA instruction: high: %d low: %d",highBits,lowBits);
                return (true);
            }
        }
        return (false);
    }

    default:
            return (false);
    }


}


cf_table_entry* dwarf_getCFEntry(void* address) {

    // find the FDE for this address range
    FDE_Header* fde = dwarf_getFDE(address,&cie);
    if (fde == 0) {
        LOG(ARCH,DEBUG,"DWARF: Could not find FDE Entry for address %x",address);
        return (0);
    }

    LOG(ARCH,DEBUG,"DWARF: FDE Entry found at %x",fde);

    /* initialize entry */
    entry.location   = (void*) fde->location;
    entry.cfa_offset = 0;
    for (int i = 0; i < 16; i++)
        entry.stored_regs[i] =  0xffffffff;

    /* get call frame instruction stream start */
    unint1* cfi_stream = ((unint1*) fde) + 16;
    /* interpret program */
    while (entry.location <= address && ((size_t) cfi_stream < (size_t) fde + fde->length +4 )) {
        if (!dwarf_interpret_cfa(&entry,cfi_stream,&cie))
            return (0);
    }

    /* got entry here */

    return (&entry);
}


char* getMethodSignature(unint4 address) {
    if (debug_strtable_entries > 0) {
        /* parse table */
        for (unsigned int i = 0; i < debug_strtable_entries; i++) {
            if (debug_strtable[i].address <= address &&
                debug_strtable[i].address + debug_strtable[i].length >= address) {
                return (debug_strtable[i].signature);
            }
        }
    }

    return ("???");
}

extern void memdump(int addr, int length);


void backtrace_addr(void* currentAddress, size_t stackPtr) {
   LOG(KERNEL,ERROR,"Backtrace:");
   LOG(KERNEL,ERROR,"  0x%08x   %s",currentAddress, getMethodSignature((unint4) currentAddress));

   cf_table_entry* cf_entry = dwarf_getCFEntry(currentAddress);
   if (cf_entry == 0) return;

   while (cf_entry) {
       stackPtr        =  stackPtr + cf_entry->cfa_offset;

       if (cf_entry->stored_regs[14] != 0xffffffff)
           currentAddress  = (void*) *( (unint4*)  ((size_t)stackPtr + cf_entry->stored_regs[14]));
       else
           return ;

       //printf("SP: %x\r",stackPtr);
       LOG(KERNEL,ERROR,"  0x%08x   %s",currentAddress, getMethodSignature((unint4) currentAddress));
       cf_entry = dwarf_getCFEntry(currentAddress);
   }
}

void backtrace_current() {

    void* currentAddress;
    size_t stackPtr;
    GETSTACKPTR(stackPtr);
    void* pc;
    GETPC(pc);
    currentAddress = pc;
    LOG(KERNEL,ERROR,"Backtrace:");

    cf_table_entry* cf_entry = dwarf_getCFEntry(currentAddress);
    if (cf_entry == 0) return;

   while (cf_entry) {
       stackPtr        =  stackPtr + cf_entry->cfa_offset;

       if (cf_entry->stored_regs[14] != 0xffffffff)
           currentAddress  = (void*) *( (unint4*)  ((size_t)stackPtr + cf_entry->stored_regs[14]));
       else
           return ;

       LOG(KERNEL,ERROR,"  0x%08x   %s",currentAddress, getMethodSignature((unint4) currentAddress));
       cf_entry = dwarf_getCFEntry(currentAddress);
   }
}


void backtrace(void** buffer, int length) {

    void* currentAddress;
    size_t stackPtr;
    GETSTACKPTR(stackPtr);
    void* pc;
    GETPC(pc);
    currentAddress = pc;

    int num = 0;

      cf_table_entry* cf_entry = dwarf_getCFEntry(currentAddress);
      if (cf_entry == 0) return;

     while (cf_entry) {
         stackPtr  =  stackPtr + cf_entry->cfa_offset;

         if (cf_entry->stored_regs[14] != 0xffffffff)
             currentAddress  = (void*) *( (unint4*)  ((size_t)stackPtr + cf_entry->stored_regs[14]));
         else
             return ;

         buffer[num++] = currentAddress;
         if (num >= length) return;

         cf_entry = dwarf_getCFEntry(currentAddress);
     }
}

#else

void dwarf_init() {

}

void backtrace_addr(void* currentAddress, size_t stackPtr) {

}

#endif
