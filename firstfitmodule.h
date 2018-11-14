typedef union header Header;

 void display_block (Header *curr);
//static void display_block (void *curr);
 void dump_freelist ();

 void *more_heap (unsigned nunits);

 int coalesce (Header *curr);
//static int coalesce (void *curr);
int init_freelist();

void *do_malloc (int nbytes);

void *mallocff (int nbytes);

void do_free (void *ptr);

void freeff (void *ptr);
