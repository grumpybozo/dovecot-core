#ifndef IMAP_EXPUNGE_H
#define IMAP_EXPUNGE_H

struct mail_search_arg;

int imap_expunge(struct mailbox *box, struct mail_search_arg *next_search_arg,
		 unsigned int *expunged_count)
	ATTR_NULL(2);

#endif
