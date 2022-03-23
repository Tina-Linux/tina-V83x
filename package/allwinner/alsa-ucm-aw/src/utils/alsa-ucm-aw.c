#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <getopt.h>
#include "common.h"
#include "alsa-ucm-aw.h"

struct context {
    const char *card_name;
    const char *verb_name;
    const char *dev_name;
    const char *mod_name;
    enum behaviour behav;
    int is_verbose;
};

struct commands_handler {
    const char *command;
    int (*func)(int argc, char *argv[], int optind, struct context *ctx);
};

static void print_help_msg(void)
{
    printf("\n");
    printf("USAGE:\n");
    printf("\talsa-ucm-aw [OPTIONS] <COMMNAD>\n");
    printf("OPTIONS:\n");
    printf("\t-h,--help                 : print help messages\n");
    printf("\t-c,--card NAME            : select the card NAME\n");
    printf("\t-v,--verb NAME            : select the verb NAME\n");
    printf("\t-d,--device NAME          : select the device NAME\n");
    printf("\t-m,--modifier NAME        : select the modifier NAME\n");
    printf("\t-p,--play                 : select \"PLAY\" behaviour\n");
    printf("\t-r,--record               : select \"RECORD\" behaviour\n");
    printf("\t\t(\"-p\" and \"-r\" are mutally exclusive. If both of them are specified,\n");
    printf("\t\t only the latter one is effective)\n");
    printf("\t-V,--verbose              : show verbose message\n");
    printf("COMMAND:\n");
    printf("\tlist              : list all cards, verbs, devices and modifiers\n");
    printf("\t                    ([c]: card, [v]: verb, [d]: device, [m]: modifier)\n");
    printf("\tenable            : run the enable sequence\n");
    printf("\tdisable           : run the disable sequence\n");
#if ENABLE_RESET_COMMAND
    printf("\treset             : reset sound card to default state\n");
#endif /* if ENABLE_RESET_COMMAND */
    printf("\tget IDENTIFIER    : get the value of a identifier in UCM configuration files\n");
    printf("\tgetvol            : get volume of a specific device\n");
    printf("\tsetvol [VALUE]    : set volume of a specific device\n");
    printf("\t                    (if not specify VALUE, it will set volume to the default\n");
    printf("\t                     value defined in UCM configuration files)\n");
    printf("EXIT CODE:\n");
    printf("\t0         : success\n");
    printf("\totherwise : error\n");
}

static void print_verbose_msg(int is_verbose, const char *fmt, ...)
{
    if (is_verbose) {
        va_list va;
        va_start(va, fmt);
        vfprintf(stdout, fmt, va);
        va_end(va);
    }
}

static int command_list_handler(int argc, char *argv[], int optind,
                                struct context *ctx)
{
    return print_all_list();
}

static int command_enable_handler(int argc, char *argv[], int optind,
                                  struct context *ctx)
{
    int ret = 0;
    const char *card = NULL;
    const char *verb = NULL;
    const char *dev = NULL;
    const char *mod = NULL;
    struct use_case_names *default_names = NULL;

    if (!(ctx->card_name && ctx->verb_name && (ctx->dev_name || ctx->mod_name))) {
        print_verbose_msg(ctx->is_verbose,
                "Not all necessary names specified. Finding default names...\n");
        if (ctx->behav == NONE) {
            fprintf(stderr, "[Error] Failed to find default names. Please use \"-p\" for"
                    " \"PLAY\" behaviour or \"-r\" for \"RECORD\" behaviour\n");
            return -1;
        }
        if (0 != default_names_init(
                    ctx->card_name, ctx->verb_name, ctx->dev_name, ctx->mod_name,
                    &default_names, ctx->behav)) {
            fprintf(stderr, "[Error] Failed to init default names from "
                    "[c] %s, [v] %s, [d] %s, [m] %s\n",
                    ctx->card_name, ctx->verb_name, ctx->dev_name, ctx->mod_name);
            return -1;
        }
        if (default_names) {
            card = default_names->card;
            verb = default_names->verb;
            dev = default_names->dev;
            mod = default_names->mod;
        }
    } else {
        card = ctx->card_name;
        verb = ctx->verb_name;
        dev = ctx->dev_name;
        mod = ctx->mod_name;
    }
    print_verbose_msg(ctx->is_verbose,
            "Enabling [c] %s, [v] %s, [d] %s, [m] %s\n", card, verb, dev, mod);
    ret = aua_use_case_enable(card, verb, dev, mod);
    if (ret == 0) {
        print_verbose_msg(ctx->is_verbose, "Successful\n");
    } else {
        print_verbose_msg(ctx->is_verbose, "Failed\n");
    }
    default_names_free(default_names);
    return ret;
}

static int command_disable_handler(int argc, char *argv[], int optind,
                                   struct context *ctx)
{
    int ret = 0;
    const char *card = NULL;
    const char *verb = NULL;
    const char *dev = NULL;
    const char *mod = NULL;
    struct use_case_names *default_names = NULL;

    if (!(ctx->card_name && ctx->verb_name && (ctx->dev_name || ctx->mod_name))) {
        print_verbose_msg(ctx->is_verbose,
                "Not all necessary names specified. Finding default names...\n");
        if (ctx->behav == NONE) {
            fprintf(stderr, "[Error] Failed to find default names. Please use \"-p\" for"
                    " \"PLAY\" behaviour or \"-r\" for \"RECORD\" behaviour\n");
            return -1;
        }
        if (0 != default_names_init(
                    ctx->card_name, ctx->verb_name, ctx->dev_name, ctx->mod_name,
                    &default_names, ctx->behav)) {
            fprintf(stderr, "[Error] Failed to init default names from "
                    "[c] %s, [v] %s, [d] %s, [m] %s\n",
                    ctx->card_name, ctx->verb_name, ctx->dev_name, ctx->mod_name);
            return -1;
        }
        if (default_names) {
            card = default_names->card;
            verb = default_names->verb;
            dev = default_names->dev;
            mod = default_names->mod;
        }
    } else {
        card = ctx->card_name;
        verb = ctx->verb_name;
        dev = ctx->dev_name;
        mod = ctx->mod_name;
    }
    print_verbose_msg(ctx->is_verbose,
            "Disabling [c] %s, [v] %s, [d] %s, [m] %s\n", card, verb, dev, mod);
    ret = aua_use_case_disable(card, verb, dev, mod);
    if (ret == 0) {
        print_verbose_msg(ctx->is_verbose, "Successful\n");
    } else {
        print_verbose_msg(ctx->is_verbose, "Failed\n");
    }
    default_names_free(default_names);
    return ret;
}

#if ENABLE_RESET_COMMAND
static int command_reset_handler(int argc, char *argv[], int optind,
                                 struct context *ctx)
{
    int ret = 0;
    const char *card = NULL;
    struct use_case_names *default_names = NULL;

    if (!ctx->card_name) {
        print_verbose_msg(ctx->is_verbose,
                "Card name not specified. Finding default names...\n");
        if (ctx->behav == NONE) {
            fprintf(stderr, "[Error] Failed to find default names. Please use \"-p\" for"
                    " \"PLAY\" behaviour or \"-r\" for \"RECORD\" behaviour\n");
            return -1;
        }
        if (0 != default_names_init(
                    ctx->card_name, ctx->verb_name, ctx->dev_name, ctx->mod_name,
                    &default_names, ctx->behav)) {
            fprintf(stderr, "[Error] Failed to init default names from "
                    "[c] %s, [v] %s, [d] %s, [m] %s\n",
                    ctx->card_name, ctx->verb_name, ctx->dev_name, ctx->mod_name);
            return -1;
        }
        if (default_names) {
            card = default_names->card;
        }
    } else {
        card = ctx->card_name;
    }
    print_verbose_msg(ctx->is_verbose, "Reseting [c] %s\n", card);
    ret = aua_use_case_reset(card);
    if (ret == 0) {
        print_verbose_msg(ctx->is_verbose, "Successful\n");
    } else {
        print_verbose_msg(ctx->is_verbose, "Failed\n");
    }
    default_names_free(default_names);
    return ret;
}
#endif /* if ENABLE_RESET_COMMAND */

static int command_get_handler(int argc, char *argv[], int optind,
                               struct context *ctx)
{
    int ret = 0;
    const char *card = NULL;
    struct use_case_names *default_names = NULL;
    if (!ctx->card_name) {
        print_verbose_msg(ctx->is_verbose,
                "Card name not specified. Finding default names...\n");
        if (ctx->behav == NONE) {
            fprintf(stderr, "[Error] Failed to find default names. Please use \"-p\" for"
                    " \"PLAY\" behaviour or \"-r\" for \"RECORD\" behaviour\n");
            ret = -1;
            goto out;
        }
        if (0 != default_names_init(
                    ctx->card_name, ctx->verb_name, ctx->dev_name, ctx->mod_name,
                    &default_names, ctx->behav)) {
            fprintf(stderr, "[Error] Failed to init default names from "
                    "[c] %s, [v] %s, [d] %s, [m] %s\n",
                    ctx->card_name, ctx->verb_name, ctx->dev_name, ctx->mod_name);
            ret = -1;
            goto out;
        }
        if (default_names) {
            card = default_names->card;
        }
    } else {
        card = ctx->card_name;
    }
    const char *verb = ctx->verb_name;
    const char *dev_or_mod = ctx->dev_name ? ctx->dev_name : ctx->mod_name;
    char *value = NULL;
    if (argc - optind <= 1) {
        fprintf(stderr, "[Error] Please input the name of value identifier\n");
        ret = -1;
        goto free_default_names;
    }
    const char *value_name = argv[optind + 1];
    value = aua_value_get(value_name, card, verb, dev_or_mod);
    if (value == NULL) {
        ret = -1;
        goto free_default_names;
    }
    print_verbose_msg(ctx->is_verbose, "The %s of %s|%s|%s: ", value_name,
            card, verb, dev_or_mod);
    fprintf(stdout, "%s\n", value);
    free(value);
    ret = 0;
free_default_names:
    default_names_free(default_names);
out:
    return ret;
}

static int command_getvol_handler(int argc, char *argv[], int optind,
                                  struct context *ctx)
{
    int ret = 0;
    const char *card = NULL;
    const char *verb = NULL;
    const char *dev = NULL;
    const char *mod = NULL;
    struct use_case_names *default_names = NULL;

    if (ctx->behav == NONE) {
        fprintf(stderr, "[Error] Please use \"-p\" for \"PLAY\" behaviour or"
                " \"-r\" for \"RECORD\" behaviour\n");
        ret = -1;
        return ret;
    }
    if (!(ctx->card_name && ctx->verb_name && (ctx->dev_name || ctx->mod_name))) {
        print_verbose_msg(ctx->is_verbose,
                "Not all necessary names specified. Finding default names...\n");
        if (0 != default_names_init(
                    ctx->card_name, ctx->verb_name, ctx->dev_name, ctx->mod_name,
                    &default_names, ctx->behav)) {
            fprintf(stderr, "[Error] Failed to init default names from "
                    "[c] %s, [v] %s, [d] %s, [m] %s\n",
                    ctx->card_name, ctx->verb_name, ctx->dev_name, ctx->mod_name);
            ret = -1;
            goto out;
        }
        if (default_names) {
            card = default_names->card;
            verb = default_names->verb;
            dev = default_names->dev;
            mod = default_names->mod;
        }
    } else {
        card = ctx->card_name;
        verb = ctx->verb_name;
        dev = ctx->dev_name;
        mod = ctx->mod_name;
    }
    const char *dev_or_mod = dev ? dev : mod;
    int volume = 0;
    switch (ctx->behav) {
    case PLAY:
        ret = aua_playback_volume_get(card, verb, dev_or_mod, &volume);
        break;
    case RECORD:
        ret = aua_capture_volume_get(card, verb, dev_or_mod, &volume);
        break;
    default:
        ret = -1;
        break;
    }
    if (ret != 0) {
        goto free_default_names;
    }
    print_verbose_msg(ctx->is_verbose, "The volume of %s|%s|%s: ",
            card, verb, dev_or_mod);
    fprintf(stdout, "%d\n", volume);
    ret = 0;
free_default_names:
    default_names_free(default_names);
out:
    return ret;
}

static int command_setvol_handler(int argc, char *argv[], int optind,
                                  struct context *ctx)
{
    int ret = 0;
    const char *card = NULL;
    const char *verb = NULL;
    const char *dev = NULL;
    const char *mod = NULL;
    struct use_case_names *default_names = NULL;

    if (ctx->behav == NONE) {
        fprintf(stderr, "[Error] Please use \"-p\" for \"PLAY\" behaviour or"
                " \"-r\" for \"RECORD\" behaviour\n");
        ret = -1;
        return ret;
    }
    if (!(ctx->card_name && ctx->verb_name && (ctx->dev_name || ctx->mod_name))) {
        print_verbose_msg(ctx->is_verbose,
                "Not all necessary names specified. Finding default names...\n");
        if (0 != default_names_init(
                    ctx->card_name, ctx->verb_name, ctx->dev_name, ctx->mod_name,
                    &default_names, ctx->behav)) {
            fprintf(stderr, "[Error] Failed to init default names from "
                    "[c] %s, [v] %s, [d] %s, [m] %s\n",
                    ctx->card_name, ctx->verb_name, ctx->dev_name, ctx->mod_name);
            ret = -1;
            goto out;
        }
        if (default_names) {
            card = default_names->card;
            verb = default_names->verb;
            dev = default_names->dev;
            mod = default_names->mod;
        }
    } else {
        card = ctx->card_name;
        verb = ctx->verb_name;
        dev = ctx->dev_name;
        mod = ctx->mod_name;
    }
    const char *dev_or_mod = dev ? dev : mod;
    if (argc - optind <= 1) {
        print_verbose_msg(ctx->is_verbose,
                "Set volume of %s|%s|%s to default\n", card, verb, dev_or_mod);
        switch (ctx->behav) {
        case PLAY:
            ret = aua_playback_volume_set_default(card, verb, dev_or_mod);
            break;
        case RECORD:
            ret = aua_capture_volume_set_default(card, verb, dev_or_mod);
            break;
        default:
            ret = -1;
            break;
        }
    } else {
        int volume = atoi(argv[optind + 1]);
        print_verbose_msg(ctx->is_verbose,
                "Set volume of %s|%s|%s to %d\n", card, verb, dev_or_mod, volume);
        switch (ctx->behav) {
        case PLAY:
            ret = aua_playback_volume_set(card, verb, dev_or_mod, volume);
            break;
        case RECORD:
            ret = aua_capture_volume_set(card, verb, dev_or_mod, volume);
            break;
        default:
            ret = -1;
            break;
        }
    }
    default_names_free(default_names);
out:
    return ret;
}

static struct commands_handler commands_handlers[] = {
    {"list", command_list_handler},
    {"enable", command_enable_handler},
    {"disable", command_disable_handler},
#if ENABLE_RESET_COMMAND
    {"reset", command_reset_handler},
#endif /* if ENABLE_RESET_COMMAND */
    {"get", command_get_handler},
    {"getvol", command_getvol_handler},
    {"setvol", command_setvol_handler},
};

int main(int argc, char *argv[])
{
    int ret;
    int opt;
    const struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"card", required_argument, NULL, 'c'},
        {"verb", required_argument, NULL, 'v'},
        {"device", required_argument, NULL, 'd'},
        {"modifier", required_argument, NULL, 'm'},
        {"play", no_argument, NULL, 'p'},
        {"record", no_argument, NULL, 'r'},
        {"verbose", no_argument, NULL, 'V'},
    };
    struct context ctx = {
        .card_name = NULL,
        .verb_name = NULL,
        .dev_name = NULL,
        .mod_name = NULL,
        .behav = NONE,
        .is_verbose = 0,
    };

    while ((opt = getopt_long(argc, argv, "hc:v:d:m:prV", long_opts, NULL)) != -1) {
        switch (opt) {
        case 'h':
            print_help_msg();
            ret = EXIT_SUCCESS;
            goto out;
        case 'c':
            ctx.card_name = optarg;
            break;
        case 'v':
            ctx.verb_name = optarg;
            break;
        case 'd':
            ctx.dev_name = optarg;
            break;
        case 'm':
            ctx.mod_name = optarg;
            break;
        case 'p':
            ctx.behav = PLAY;
            break;
        case 'r':
            ctx.behav = RECORD;
            break;
        case 'V':
            ctx.is_verbose = 1;
            break;
        default:
            fprintf(stderr, "Invalid option: -%c\n", (char)opt);
            print_help_msg();
            ret = EXIT_FAILURE;
            goto out;
        }
    }

    if (argc - optind <= 0) {
        print_help_msg();
        ret = EXIT_FAILURE;
        goto out;
    }

    int err = 0;
    unsigned long i;
    for (i = 0; i < sizeof(commands_handlers) / sizeof(commands_handlers[0]); ++i) {
        if (0 == strcmp(argv[optind], commands_handlers[i].command)) {
            err = commands_handlers[i].func(argc, argv, optind, &ctx);
            break;
        }
    }
    if (err != 0) {
        ret = EXIT_FAILURE;
        goto out;
    }

    ret = EXIT_SUCCESS;

out:
    return ret;
}
