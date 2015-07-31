"""
Utility functions to create MongoDB processes.

Handles all the nitty-gritty parameter conversion.
"""

from __future__ import absolute_import

import json
import os
import os.path
import stat

from . import process as _process
from .. import utils
from .. import config


def mongod_program(logger, executable=None, process_kwargs=None, **kwargs):
    """
    Returns a Process instance that starts a mongod executable with
    arguments constructed from 'kwargs'.
    """

    executable = utils.default_if_none(executable, config.DEFAULT_MONGOD_EXECUTABLE)
    args = [executable]

    # Apply the --setParameter command line argument. Command line options to resmoke.py override
    # the YAML configuration.
    suite_set_parameters = kwargs.pop("set_parameters", {})

    if config.MONGOD_SET_PARAMETERS is not None:
        suite_set_parameters.update(utils.load_yaml(config.MONGOD_SET_PARAMETERS))

    _apply_set_parameters(args, suite_set_parameters)

    shortcut_opts = {
        "nojournal": config.NO_JOURNAL,
        "nopreallocj": config.NO_PREALLOC_JOURNAL,
        "storageEngine": config.STORAGE_ENGINE,
        "wiredTigerCollectionConfigString": config.WT_COLL_CONFIG,
        "wiredTigerEngineConfigString": config.WT_ENGINE_CONFIG,
        "wiredTigerIndexConfigString": config.WT_INDEX_CONFIG,
    }

    # These options are just flags, so they should not take a value.
    opts_without_vals = ("nojournal", "nopreallocj")

    # Have the --nojournal command line argument to resmoke.py unset the journal option.
    if shortcut_opts["nojournal"] is not None and "journal" in kwargs:
        del kwargs["journal"]

    for opt_name in shortcut_opts:
        if shortcut_opts[opt_name] is not None:
            # Command line options override the YAML configuration.
            if opt_name in opts_without_vals:
                kwargs[opt_name] = ""
            else:
                kwargs[opt_name] = shortcut_opts[opt_name]

    # Apply the rest of the command line arguments.
    _apply_kwargs(args, kwargs)

    _set_keyfile_permissions(kwargs)

    process_kwargs = utils.default_if_none(process_kwargs, {})
    return _process.Process(logger, args, **process_kwargs)


def mongos_program(logger, executable=None, process_kwargs=None, **kwargs):
    """
    Returns a Process instance that starts a mongos executable with
    arguments constructed from 'kwargs'.
    """

    executable = utils.default_if_none(executable, config.DEFAULT_MONGOS_EXECUTABLE)
    args = [executable]

    # Apply the --setParameter command line argument. Command line options to resmoke.py override
    # the YAML configuration.
    suite_set_parameters = kwargs.pop("set_parameters", {})

    if config.MONGOS_SET_PARAMETERS is not None:
        suite_set_parameters.update(utils.load_yaml(config.MONGOS_SET_PARAMETERS))

    _apply_set_parameters(args, suite_set_parameters)

    # Apply the rest of the command line arguments.
    _apply_kwargs(args, kwargs)

    _set_keyfile_permissions(kwargs)

    process_kwargs = utils.default_if_none(process_kwargs, {})
    return _process.Process(logger, args, **process_kwargs)


def mongo_shell_program(logger, executable=None, filename=None, process_kwargs=None, **kwargs):
    """
    Returns a Process instance that starts a mongo shell with arguments
    constructed from 'kwargs'.
    """

    executable = utils.default_if_none(executable, config.DEFAULT_MONGO_EXECUTABLE)
    args = [executable]

    eval_sb = []  # String builder.
    global_vars = kwargs.pop("global_vars", {})

    shortcut_opts = {
        "noJournal": (config.NO_JOURNAL, False),
        "noJournalPrealloc": (config.NO_PREALLOC_JOURNAL, False),
        "storageEngine": (config.STORAGE_ENGINE, ""),
        "testName": (os.path.splitext(os.path.basename(filename))[0], ""),
        "wiredTigerCollectionConfigString": (config.WT_COLL_CONFIG, ""),
        "wiredTigerEngineConfigString": (config.WT_ENGINE_CONFIG, ""),
        "wiredTigerIndexConfigString": (config.WT_INDEX_CONFIG, ""),
    }

    test_data = global_vars.get("TestData", {}).copy()
    for opt_name in shortcut_opts:
        (opt_value, opt_default) = shortcut_opts[opt_name]
        if opt_value is not None:
            test_data[opt_name] = opt_value
        elif opt_name not in test_data:
            # Only use 'opt_default' if the property wasn't set in the YAML configuration.
            test_data[opt_name] = opt_default
    global_vars["TestData"] = test_data

    for var_name in global_vars:
        _format_shell_vars(eval_sb, var_name, global_vars[var_name])

    if "eval" in kwargs:
        eval_sb.append(kwargs.pop("eval"))

    eval_str = "; ".join(eval_sb)
    args.append("--eval")
    args.append(eval_str)

    if config.SHELL_WRITE_MODE is not None:
        kwargs["writeMode"] = config.SHELL_WRITE_MODE

    # Apply the rest of the command line arguments.
    _apply_kwargs(args, kwargs)

    # Have the mongos shell run the specified file.
    args.append(filename)

    _set_keyfile_permissions(global_vars["TestData"])

    process_kwargs = utils.default_if_none(process_kwargs, {})
    return _process.Process(logger, args, **process_kwargs)


def _format_shell_vars(sb, path, value):
    """
    Formats 'value' in a way that can be passed to --eval.

    If 'value' is a dictionary, then it is unrolled into the creation of
    a new JSON object with properties assigned for each key of the
    dictionary.
    """

    # Only need to do special handling for JSON objects.
    if not isinstance(value, dict):
        sb.append("%s = %s" % (path, json.dumps(value)))
        return

    # Avoid including curly braces and colons in output so that the command invocation can be
    # copied and run through bash.
    sb.append("%s = new Object()" % (path))
    for subkey in value:
        _format_shell_vars(sb, ".".join((path, subkey)), value[subkey])


def dbtest_program(logger, executable=None, suites=None, process_kwargs=None, **kwargs):
    """
    Returns a Process instance that starts a dbtest executable with
    arguments constructed from 'kwargs'.
    """

    executable = utils.default_if_none(executable, config.DEFAULT_DBTEST_EXECUTABLE)
    args = [executable]

    if suites is not None:
        args.extend(suites)

    if config.STORAGE_ENGINE is not None:
        kwargs["storageEngine"] = config.STORAGE_ENGINE

    for arg_name in kwargs:
        arg_value = str(kwargs[arg_name])
        args.append("--%s" % (arg_name))
        if arg_value:
            args.append(arg_value)

    process_kwargs = utils.default_if_none(process_kwargs, {})
    return _process.Process(logger, args, **process_kwargs)


def _apply_set_parameters(args, set_parameter):
    """
    Converts key-value pairs from 'kwargs' into --setParameter key=value
    arguments to an executable and appends them to 'args'.
    """

    for param_name in set_parameter:
        param_value = set_parameter[param_name]
        # --setParameter takes boolean values as lowercase strings.
        if isinstance(param_value, bool):
            param_value = "true" if param_value else "false"
        args.append("--setParameter")
        args.append("%s=%s" % (param_name, param_value))


def _apply_kwargs(args, kwargs):
    """
    Converts key-value pairs from 'kwargs' into --key value arguments
    to an executable and appends them to 'args'.

    A --flag without a value is represented with the empty string.
    """

    for arg_name in kwargs:
        arg_value = str(kwargs[arg_name])
        args.append("--%s" % (arg_name))
        if arg_value:
            args.append(arg_value)


def _set_keyfile_permissions(opts):
    """
    Change the permissions of keyfiles in 'opts' to 600, i.e. only the
    user can read and write the file.

    This necessary to avoid having the mongod/mongos fail to start up
    because "permissions on the keyfiles are too open".

    We can't permanently set the keyfile permissions because git is not
    aware of them.
    """
    if "keyFile" in opts:
        os.chmod(opts["keyFile"], stat.S_IRUSR | stat.S_IWUSR)
    if "encryptionKeyFile" in opts:
        os.chmod(opts["encryptionKeyFile"], stat.S_IRUSR | stat.S_IWUSR)
