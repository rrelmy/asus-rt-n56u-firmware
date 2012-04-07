/* include/config.h.  Generated automatically by configure.  */
/* Define if you have the attr/error_context.h header */
/* #undef HAVE_ATTR_ERROR_CONTEXT_H */

/* Define if you want Posix compliant getfacl and setfacl utilities
   without extensions */
/* #undef POSIXLY_CORRECT */

/* The number of bytes in a int.  */
/* #undef SIZEOF_INT */

/* The number of bytes in a long.  */
/* #undef SIZEOF_LONG */

/* The number of bytes in a short.  */
/* #undef SIZEOF_SHORT */

/* Define if you have attribute((visibility(hidden))) in gcc. */
#define HAVE_VISIBILITY_ATTRIBUTE 1

/* Define if you want gettext (I18N) support */
/* #undef ENABLE_GETTEXT */

#ifdef ENABLE_GETTEXT
# include <libintl.h>
# define _(x)			gettext(x)
#else
# define _(x)			(x)
# define textdomain(d)		do { } while (0)
# define bindtextdomain(d,dir)	do { } while (0)
#endif
#include <locale.h>

