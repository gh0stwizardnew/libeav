#ifndef EAV_H
#define EAV_H

#include <idn/api.h>
#include <stdbool.h>


typedef int EAV_RFC;
enum {
    EAV_RFC_822,
    EAV_RFC_5321,
    EAV_RFC_5322,
    EAV_RFC_6531,
};

/* XXX conflicts with auto-gerated?? */
typedef int EAV_TLD;
enum {
    EAV_TLD_INVALID             = 1 << 1,
    EAV_TLD_NOT_ASSIGNED        = 1 << 2,
    EAV_TLD_COUNTRY_CODE        = 1 << 3,
    EAV_TLD_GENERIC             = 1 << 4,
    EAV_TLD_GENERIC_RESTRICTED  = 1 << 5,
    EAV_TLD_INFRASTRUCTURE      = 1 << 6,
    EAV_TLD_SPONSORED           = 1 << 7,
    EAV_TLD_TEST                = 1 << 8,
};

typedef int (*eav_utf8_f)
            (idn_resconf_t, idn_action_t, const char *, size_t);
typedef int (*eav_ascii_f) (const char *, size_t);

typedef struct eav_s {
    int             rfc;        /* choose rfc */
    int             allow_tld;  /* flag: allow only these TLDs */
    bool            utf8;       /* allow utf8 */
    bool            tld_check;  /* do fqdn & tld checks */
    const char      *errmsg;
    /* XXX private */
    idn_resconf_t   idn;
    idn_action_t    actions;
    eav_utf8_f      utf8_cb;
    eav_ascii_f     ascii_cb;    
} eav_t;

/* libeav error codes */
typedef int TLD_ERROR;
enum {
    EEAV_NO_ERROR,
    EEAV_IDN_INIT_FAIL,
    EEAV_IND_CONF_FAIL,
    EEAV_INVALID_RFC,
    EEAV_EMPTY,
    EEAV_IDNKIT_ERROR,
    EEAV_NOT_ASCII,
    EEAV_SPECIAL,
    EEAV_CTRL_CHAR,
    EEAV_LPART_MISPLACED_QUOTE,
    EEAV_LPART_SPECIAL,
    EEAV_LPART_UNQUOTED,
    EEAV_LPART_TOO_MANY_DOTS,
    EEAV_LPART_UNQUOTED_FWS,
    EEAV_LPART_INVALID_UTF8,
    EEAV_DOMAIN_LABEL_TOO_LONG,
    EEAV_DOMAIN_MISPLACED_DELIMITER,
    EEAV_DOMAIN_MISPLACED_HYPHEN,
    EEAV_DOMAIN_INVALID_CHAR,
    EEAV_DOMAIN_TOO_LONG,
    EEAV_DOMAIN_NUMERIC,
    EEAV_DOMAIN_NOT_FQDN,
    EEAV_EMAIL_HAS_NO_DOMAIN,
    EEAV_LPART_TOO_LONG,
    EEAV_EMAIL_INVALID_IPADDR,
    EEAV_EMAIL_NO_PAIRED_BRACKET,
    EEAV_MAX /* debug & test only */
};


extern void
eav_init (eav_t *eav);

extern void
eav_free (eav_t *eav);

extern int
eav_setup (eav_t *eav);

extern int
eav_is_email (eav_t *eav, const char *email, size_t length);

extern const char *
eav_errstr (eav_t *eav);

extern int
is_6531_local (const char *start, const char *end);

extern int
is_5322_local (const char *start, const char *end);

extern int
is_5321_local (const char *start, const char *end);

extern int
is_822_local (const char *start, const char *end);

extern int
is_ipv4 (const char *start, const char *end);

extern int
is_ipv6 (const char *start, const char *end);

extern int
is_ascii_domain (const char *start, const char *end);

extern int
is_utf8_domain (idn_resconf_t ctx,
                idn_action_t actions,
                const char *start,
                const char *end);

extern int
is_tld (const char *start, const char *end);

extern int
is_utf8_inet_domain(idn_resconf_t ctx,
                    idn_action_t actions,
                    const char *start,
                    const char *end);

extern int
is_special_domain (const char *start, const char *end);

extern int
is_6531_email (  idn_resconf_t ctx,
                    idn_action_t actions,
                    const char *email,
                    size_t length);

extern int
is_6531_email_fqdn ( idn_resconf_t ctx,
                        idn_action_t actions,
                        const char *email,
                        size_t length);

extern int
is_email_rfc822 (const char *email, size_t length);

extern int
is_email_rfc5321 (const char *email, size_t length);

extern int
is_email_rfc5322 (const char *email, size_t length);

extern int
is_ipaddr (const char *start, const char *end);

#endif /* EAV_H */
