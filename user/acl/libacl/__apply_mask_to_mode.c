#include <sys/stat.h>
#include <acl/libacl.h>
#include "libacl.h"

#if defined(HAVE_ACL_ENTRIES) && \
    defined (HAVE_ACL_GET_ENTRY) && defined(HAVE_ACL_GET_TAG_TYPE) && \
    defined (HAVE_ACL_GET_PERMSET) && defined(HAVE_ACL_GET_PERM)
int
__apply_mask_to_mode(mode_t *mode, acl_t acl)
{
	acl_entry_t entry;
	int entry_id=ACL_FIRST_ENTRY;

	/* A mimimal ACL which has three entries has no mask entry; the
	   group file mode permission bits are exact. */
	if (acl_entries(acl) == 3)
		return 0;

	while (acl_get_entry(acl, entry_id, &entry) == 1) {
		acl_tag_t tag_type;

		acl_get_tag_type(entry, &tag_type);
		if (tag_type == ACL_MASK) {
			acl_permset_t permset;

			acl_get_permset(entry, &permset);
			if (acl_get_perm(permset, ACL_READ) != 1)
				*mode &= ~S_IRGRP;
			if (acl_get_perm(permset, ACL_WRITE) != 1)
				*mode &= ~S_IWGRP;
			if (acl_get_perm(permset, ACL_EXECUTE) != 1)
				*mode &= ~S_IXGRP;

			return 0;
		}
		entry_id = ACL_NEXT_ENTRY;
	}

	/* This is unexpected; if the ACL didn't include a mask entry
	   we should have exited before the loop! */
	*mode &= ~S_IRWXG;
	return 1;
}
#endif


