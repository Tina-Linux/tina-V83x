#include <alsa/asoundlib.h>

#define LEVEL_BASIC		(1<<0)
#define LEVEL_INACTIVE		(1<<1)
#define LEVEL_ID		(1<<2)

static char card[64] = "default";
static int quiet = 0;
static int debugflag = 0;
static int no_check = 0;
static int smixer_level = 0;
static int ignore_error = 0;
static void print_spaces(unsigned int spaces)
{
	while (spaces-- > 0)
		putc(' ', stdout);
}

static void print_dB(long dB)
{
	if (dB < 0) {
		printf("-%li.%02lidB", -dB / 100, -dB % 100);
	} else {
		printf("%li.%02lidB", dB / 100, dB % 100);
	}
}
static const char *control_type(snd_ctl_elem_info_t *info)
{
	return snd_ctl_elem_type_name(snd_ctl_elem_info_get_type(info));
}
static void show_control_id(snd_ctl_elem_id_t *id)
{
	char *str;

	str = snd_ctl_ascii_elem_id_get(id);
	if (str)
		printf("%s", str);
	free(str);
}
static const char *control_access(snd_ctl_elem_info_t *info)
{
	static char result[10];
	char *res = result;

	*res++ = snd_ctl_elem_info_is_readable(info) ? 'r' : '-';
	*res++ = snd_ctl_elem_info_is_writable(info) ? 'w' : '-';
	*res++ = snd_ctl_elem_info_is_inactive(info) ? 'i' : '-';
	*res++ = snd_ctl_elem_info_is_volatile(info) ? 'v' : '-';
	*res++ = snd_ctl_elem_info_is_locked(info) ? 'l' : '-';
	*res++ = snd_ctl_elem_info_is_tlv_readable(info) ? 'R' : '-';
	*res++ = snd_ctl_elem_info_is_tlv_writable(info) ? 'W' : '-';
	*res++ = snd_ctl_elem_info_is_tlv_commandable(info) ? 'C' : '-';
	*res++ = '\0';
	return result;
}
static void error(const char *fmt,...)
{
	va_list va;

	va_start(va, fmt);
	fprintf(stderr, "amixer: ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	va_end(va);
}
static void decode_tlv(unsigned int spaces, unsigned int *tlv, unsigned int tlv_size)
{
	unsigned int type = tlv[0];
	unsigned int size;
	unsigned int idx = 0;
	const char *chmap_type = NULL;

	if (tlv_size < 2 * sizeof(unsigned int)) {
		printf("TLV size error!\n");
		return;
	}
	print_spaces(spaces);
	printf("| ");
	type = tlv[idx++];
	size = tlv[idx++];
	tlv_size -= 2 * sizeof(unsigned int);
	if (size > tlv_size) {
		printf("TLV size error (%u, %u, %u)!\n", type, size, tlv_size);
		return;
	}
	switch (type) {
	case SND_CTL_TLVT_CONTAINER:
		printf("container\n");
		size += sizeof(unsigned int) -1;
		size /= sizeof(unsigned int);
		while (idx < size) {
			if (tlv[idx+1] > (size - idx) * sizeof(unsigned int)) {
				printf("TLV size error in compound!\n");
				return;
			}
			decode_tlv(spaces + 2, tlv + idx, tlv[idx+1] + 8);
			idx += 2 + (tlv[idx+1] + sizeof(unsigned int) - 1) / sizeof(unsigned int);
		}
		break;
	case SND_CTL_TLVT_DB_SCALE:
		printf("dBscale-");
		if (size != 2 * sizeof(unsigned int)) {
			while (size > 0) {
				printf("0x%08x,", tlv[idx++]);
				size -= sizeof(unsigned int);
			}
		} else {
			printf("min=");
			print_dB((int)tlv[2]);
			printf(",step=");
			print_dB(tlv[3] & 0xffff);
			printf(",mute=%i", (tlv[3] >> 16) & 1);
		}
		break;
	default:
		printf("unk-%u-", type);
		while (size > 0) {
			printf("0x%08x,", tlv[idx++]);
			size -= sizeof(unsigned int);
		}
		break;
	}
	putc('\n', stdout);
}
static int show_control(const char *space, snd_hctl_elem_t *elem,
			int level)
{
	int err;
	unsigned int item, idx, count, *tlv;
	snd_ctl_elem_type_t type;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_value_t *control;
	snd_aes_iec958_t iec958;
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_value_alloca(&control);
	if ((err = snd_hctl_elem_info(elem, info)) < 0) {
		error("Control %s snd_hctl_elem_info error: %s\n", card, snd_strerror(err));
		return err;
	}
	if (level & LEVEL_ID) {
		snd_hctl_elem_get_id(elem, id);
		show_control_id(id);
		printf("\n");
	}
	count = snd_ctl_elem_info_get_count(info);
	type = snd_ctl_elem_info_get_type(info);
	printf("%s; type=%s,access=%s,values=%u", space, control_type(info), control_access(info), count);
	switch (type) {
	case SND_CTL_ELEM_TYPE_INTEGER:
		printf(",min=%li,max=%li,step=%li\n",
		       snd_ctl_elem_info_get_min(info),
		       snd_ctl_elem_info_get_max(info),
		       snd_ctl_elem_info_get_step(info));
		break;
	case SND_CTL_ELEM_TYPE_INTEGER64:
		printf(",min=%Li,max=%Li,step=%Li\n",
		       snd_ctl_elem_info_get_min64(info),
		       snd_ctl_elem_info_get_max64(info),
		       snd_ctl_elem_info_get_step64(info));
		break;
	case SND_CTL_ELEM_TYPE_ENUMERATED:
	{
		unsigned int items = snd_ctl_elem_info_get_items(info);
		printf(",items=%u\n", items);
		for (item = 0; item < items; item++) {
			snd_ctl_elem_info_set_item(info, item);
			if ((err = snd_hctl_elem_info(elem, info)) < 0) {
				error("Control %s element info error: %s\n", card, snd_strerror(err));
				return err;
			}
			printf("%s; Item #%u '%s'\n", space, item, snd_ctl_elem_info_get_item_name(info));
		}
		break;
	}
	default:
		printf("\n");
		break;
	}
	if (level & LEVEL_BASIC) {
		if (!snd_ctl_elem_info_is_readable(info))
			goto __skip_read;
		if ((err = snd_hctl_elem_read(elem, control)) < 0) {
			error("Control %s element read error: %s\n", card, snd_strerror(err));
			return err;
		}
		printf("%s: values=", space);
		for (idx = 0; idx < count; idx++) {
			if (idx > 0)
				printf(",");
			switch (type) {
			case SND_CTL_ELEM_TYPE_BOOLEAN:
				printf("%s", snd_ctl_elem_value_get_boolean(control, idx) ? "on" : "off");
				break;
			case SND_CTL_ELEM_TYPE_INTEGER:
				printf("%li", snd_ctl_elem_value_get_integer(control, idx));
				break;
			case SND_CTL_ELEM_TYPE_INTEGER64:
				printf("%Li", snd_ctl_elem_value_get_integer64(control, idx));
				break;
			case SND_CTL_ELEM_TYPE_ENUMERATED:
				printf("%u", snd_ctl_elem_value_get_enumerated(control, idx));
				break;
			case SND_CTL_ELEM_TYPE_BYTES:
				printf("0x%02x", snd_ctl_elem_value_get_byte(control, idx));
				break;
			case SND_CTL_ELEM_TYPE_IEC958:
				snd_ctl_elem_value_get_iec958(control, &iec958);
				printf("[AES0=0x%02x AES1=0x%02x AES2=0x%02x AES3=0x%02x]",
				       iec958.status[0], iec958.status[1],
				       iec958.status[2], iec958.status[3]);
				break;
			default:
				printf("?");
				break;
			}
		}
		printf("\n");
	      __skip_read:
		if (!snd_ctl_elem_info_is_tlv_readable(info))
			goto __skip_tlv;
		tlv = malloc(4096);
		if ((err = snd_hctl_elem_tlv_read(elem, tlv, 4096)) < 0) {
			error("Control %s element TLV read error: %s\n", card, snd_strerror(err));
			free(tlv);
			return err;
		}
		decode_tlv(strlen(space), tlv, 4096);
		free(tlv);
	}
      __skip_tlv:
	return 0;
}

int cset(char* device,int argc, char *argv[], int roflag, int keep_handle)
{
	int err;
	static snd_ctl_t *handle = NULL;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_value_t *control;
	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_value_alloca(&control);

	if (argc < 1) {
		fprintf(stderr, "Specify a full control identifier: [[iface=<iface>,][name='name',][index=<index>,][device=<device>,][subdevice=<subdevice>]]|[numid=<numid>]\n");
		return -EINVAL;
	}

	strncpy(card,device,sizeof(card));

	if (snd_ctl_ascii_elem_id_parse(id, argv[0])) {
		fprintf(stderr, "Wrong control identifier: %s\n", argv[0]);
		return -EINVAL;
	}
	if (debugflag) {
		printf("VERIFY ID: ");
		show_control_id(id);
		printf("\n");
	}
	if (handle == NULL &&
	    (err = snd_ctl_open(&handle, card, 0)) < 0) {
		error("Control %s open error: %s\n", card, snd_strerror(err));
		return err;
	}
	snd_ctl_elem_info_set_id(info, id);
	if ((err = snd_ctl_elem_info(handle, info)) < 0) {
		if (ignore_error)
			return 0;
		error("Cannot find the given element from control %s\n", card);
		if (! keep_handle) {
			snd_ctl_close(handle);
			handle = NULL;
		}
		return err;
	}
	snd_ctl_elem_info_get_id(info, id);     /* FIXME: Remove it when hctl find works ok !!! */
	if (!roflag) {
		snd_ctl_elem_value_set_id(control, id);
		if ((err = snd_ctl_elem_read(handle, control)) < 0) {
			if (ignore_error)
				return 0;
			error("Cannot read the given element from control %s\n", card);
			if (! keep_handle) {
				snd_ctl_close(handle);
				handle = NULL;
			}
			return err;
		}
		err = snd_ctl_ascii_value_parse(handle, control, info, argv[1]);
		if (err < 0) {
			if (!ignore_error)
				error("Control %s parse error: %s\n", card, snd_strerror(err));
			if (!keep_handle) {
				snd_ctl_close(handle);
				handle = NULL;
			}
			return ignore_error ? 0 : err;
		}
		if ((err = snd_ctl_elem_write(handle, control)) < 0) {
			if (!ignore_error)
				error("Control %s element write error: %s\n", card, snd_strerror(err));
			if (!keep_handle) {
				snd_ctl_close(handle);
				handle = NULL;
			}
			return ignore_error ? 0 : err;
		}
	}
	if (! keep_handle) {
		snd_ctl_close(handle);
		handle = NULL;
	}
	if (!quiet) {
		snd_hctl_t *hctl;
		snd_hctl_elem_t *elem;
		if ((err = snd_hctl_open(&hctl, card, 0)) < 0) {
			error("Control %s open error: %s\n", card, snd_strerror(err));
			return err;
		}
		if ((err = snd_hctl_load(hctl)) < 0) {
			error("Control %s load error: %s\n", card, snd_strerror(err));
			return err;
		}
		elem = snd_hctl_find_elem(hctl, id);
		if (elem)
			show_control("  ", elem, LEVEL_BASIC | LEVEL_ID);
		else
			printf("Could not find the specified element\n");
		snd_hctl_close(hctl);
	}
	return 0;
}
