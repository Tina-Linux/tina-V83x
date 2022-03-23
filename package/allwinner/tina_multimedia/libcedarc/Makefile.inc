
CFLAGS_CDCG = -g -Wall -Werror -D_GNU_SOURCE \
			  -Wno-unused-variable \
			  -Wno-unused-but-set-variable \
			  -Wno-unused-result \
			  -Wno-unused-function \
			  -Wno-unused-value \
			  -Wno-pointer-to-int-cast \
			  -Wno-int-to-pointer-cast \
			  -Wno-unused-label \
			  -Wno-strict-aliasing \
                          -lm

CPPFLAGS_CDCG = -g -Wall -Werror -D_GNU_SOURCE \
				-Wno-unused-variable \
				-Wno-unused-but-set-variable \
			    -Wno-unused-value \
			    -Wno-unused-result \
                            -lm

LDFLAGS_CDCG = -Wl,--no-undefined -avoid-version
