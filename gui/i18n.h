#ifndef __I18N_H__
#define __I18N_H__

#define GETTEXT_PACKAGE "drnoksnes"

#if CONF_NLS
#include <libintl.h>
#define _(String) dgettext(GETTEXT_PACKAGE, String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#else /* NLS is disabled */
#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain) 
#define bind_textdomain_codeset(Domain,Codeset) (Codeset) 
#endif /* CONF_NLS */

#endif

