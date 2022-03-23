
POLICYVERS := 31
MLS_SENS=1
MLS_CATS=1024

all: policy.$(POLICYVERS)


policy.$(POLICYVERS): policy.conf
	$(CHECKPOLICY) -M -c $(POLICYVERS) -o $@ $<

policy.conf: $(wildcard security_classes initial_sids access_vectors global_macros neverallow_macros mls_macros mls policy_capabilities te_macros attributes ioctl_macros *.te roles users  initial_sid_contexts fs_use genfs_contexts port_contexts)
	$(M4) -D mls_num_sens=$(MLS_SENS) -D mls_num_cats=$(MLS_CATS) -s $^ > $@
