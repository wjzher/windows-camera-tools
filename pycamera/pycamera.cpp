#include "stdafx.h"
#include <Python.h>
#include <atlbase.h>
#include "..\include\libcamera.h"
#include "common.h"

#pragma comment(lib, "..\\out\\libcore.lib")

/*
 * Implements an set auto exposure function.
 */
PyDoc_STRVAR(pycamera_set_auto_exposure_doc, "setAutoExposure(id, number)\
\
Set Auto Exposure function");

static int SetupExposure2(LONG id, LONG state)
{
    CComPtr<ICameraDs> spCameraDs = nullptr;
    HRESULT hr = S_OK;
    int retcode = DEFAULT_ERROR;

    hr = CreateCameraDsInstance(&spCameraDs);

    if (SUCCEEDED(hr) && spCameraDs)
    {
        hr = spCameraDs->Initialize(id);

        if (SUCCEEDED(hr))
        {
            hr = spCameraDs->SetAutoExposure(state);

            if (SUCCEEDED(hr))
            {
                _tprintf(L"Output: Operation successful.\n");
                retcode = NOERROR;
            }
            else
            {
                ErrorCom(hr, L"SetAutoExposure");
                retcode = DEFAULT_ERROR;
            }
            spCameraDs->CloseInterfaces();
        }
        else
        {
            ErrorCom(hr, L"Initialize");
            retcode = DEFAULT_ERROR;
        }
    }
    return retcode;
}


PyObject * pycamera_set_auto_exposure(PyObject *self, PyObject *args)
{
    /* Shared references that do not need Py_DECREF before returning. */
    PyObject *obj = NULL;
    HRESULT hr = S_OK;
    LONG id, state;
    if (!PyArg_ParseTuple(args, "ii", &id, &state))
    {
        return PyLong_FromLong(-EINVAL);
    }
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    hr = SetupExposure2(id, state);
    CoUninitialize();
    return PyLong_FromLong(hr);
}

/*
 * List of functions to add to pycamera in exec_pycamera().
 */
static PyMethodDef pycamera_functions[] = {
    { "setAutoExposure", (PyCFunction)pycamera_set_auto_exposure, METH_VARARGS, pycamera_set_auto_exposure_doc },
    { NULL, NULL, 0, NULL } /* marks end of array */
};

/*
 * Initialize pycamera. May be called multiple times, so avoid
 * using static state.
 */
int exec_pycamera(PyObject *module) {
    PyModule_AddFunctions(module, pycamera_functions);

    PyModule_AddStringConstant(module, "__author__", "zhaozhe");
    PyModule_AddStringConstant(module, "__version__", "1.0.0");
    PyModule_AddIntConstant(module, "year", 2020);

    return 0; /* success */
}

/*
 * Documentation for pycamera.
 */
PyDoc_STRVAR(pycamera_doc, "The pycamera module");


static PyModuleDef_Slot pycamera_slots[] = {
    { Py_mod_exec, exec_pycamera },
    { 0, NULL }
};

static PyModuleDef pycamera_def = {
    PyModuleDef_HEAD_INIT,
    "pycamera",
    pycamera_doc,
    0,              /* m_size */
    NULL,           /* m_methods */
    pycamera_slots,
    NULL,           /* m_traverse */
    NULL,           /* m_clear */
    NULL,           /* m_free */
};

PyMODINIT_FUNC PyInit_pycamera() {
    return PyModuleDef_Init(&pycamera_def);
}
