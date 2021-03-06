/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod REST in Pawn Extension
 * Copyright 2017 Erik Minekus
 * =============================================================================
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "extension.h"

static cell_t CreateObject(IPluginContext *pContext, const cell_t *params)
{
	json_t *object = json_object();

	Handle_t hndl = handlesys->CreateHandle(htJSONObject, object, pContext->GetIdentity(), myself->GetIdentity(), NULL);
	if (hndl == BAD_HANDLE)
	{
		pContext->ThrowNativeError("Could not create object handle.");
		return BAD_HANDLE;
	}

	return hndl;
}

static cell_t GetObjectSize(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	return json_object_size(object);
}

static cell_t GetObjectValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
		return BAD_HANDLE;
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	json_t *value = json_object_get(object, key);
	if (value == NULL)
	{
		pContext->ThrowNativeError("Could not retrieve value for key '%s'", key);
		return BAD_HANDLE;
	}

	Handle_t hndlValue = handlesys->CreateHandleEx(htJSONObject, value, &sec, NULL, NULL);
	if (hndlValue == BAD_HANDLE)
	{
		pContext->ThrowNativeError("Could not create value handle.");
		return BAD_HANDLE;
	}

	// Increase the reference counter, meaning the value handle must be
	// freed via delete or CloseHandle().
	json_incref(value);

	return hndlValue;
}

static cell_t GetObjectBoolValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	json_t *value = json_object_get(object, key);
	if (value == NULL)
	{
		return pContext->ThrowNativeError("Could not retrieve value for key '%s'", key);
	}

	return json_boolean_value(value);
}

static cell_t GetObjectFloatValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	json_t *value = json_object_get(object, key);
	if (value == NULL)
	{
		return pContext->ThrowNativeError("Could not retrieve value for key '%s'", key);
	}

	return sp_ftoc(json_real_value(value));
}

static cell_t GetObjectIntValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	json_t *value = json_object_get(object, key);
	if (value == NULL)
	{
		return pContext->ThrowNativeError("Could not retrieve value for key '%s'", key);
	}

	return json_integer_value(value);
}

static cell_t GetObjectStringValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	json_t *value = json_object_get(object, key);
	if (value == NULL)
	{
		return pContext->ThrowNativeError("Could not retrieve value for key '%s'", key);
	}

	const char *result = json_string_value(value);
	if (result == NULL)
	{
		return 0;
	}

	pContext->StringToLocalUTF8(params[3], params[4], result, NULL);

	return 1;
}

static cell_t IsObjectNullValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	json_t *value = json_object_get(object, key);
	if (value == NULL)
	{
		return pContext->ThrowNativeError("Could not retrieve value for key '%s'", key);
	}

	return json_is_null(value);
}

static cell_t SetObjectValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	json_t *value;
	Handle_t hndlValue = static_cast<Handle_t>(params[3]);
	if ((err=handlesys->ReadHandle(hndlValue, htJSONObject, &sec, (void **)&value)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid value handle %x (error %d)", hndlValue, err);
	}

	return (json_object_set(object, key, value) == 0);
}

static cell_t SetObjectBoolValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	json_t *value = json_boolean(params[3]);

	return (json_object_set_new(object, key, value) == 0);
}

static cell_t SetObjectFloatValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	json_t *value = json_real(sp_ctof(params[3]));

	return (json_object_set_new(object, key, value) == 0);
}

static cell_t SetObjectIntValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	json_t *value = json_integer(params[3]);

	return (json_object_set_new(object, key, value) == 0);
}

static cell_t SetObjectNullValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	json_t *value = json_null();

	return (json_object_set_new(object, key, value) == 0);
}

static cell_t SetObjectStringValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	char *val;
	pContext->LocalToString(params[3], &val);

	json_t *value = json_string(val);

	return (json_object_set_new(object, key, value) == 0);
}

static cell_t RemoveFromObject(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	return (json_object_del(object, key) == 0);
}

static cell_t ClearObject(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	return (json_object_clear(object) == 0);
}

static cell_t CreateArray(IPluginContext *pContext, const cell_t *params)
{
	json_t *object = json_array();

	Handle_t hndl = handlesys->CreateHandle(htJSONObject, object, pContext->GetIdentity(), myself->GetIdentity(), NULL);
	if (hndl == BAD_HANDLE)
	{
		pContext->ThrowNativeError("Could not create array handle.");
		return BAD_HANDLE;
	}

	return hndl;
}

static cell_t GetArraySize(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	return json_array_size(object);
}

static cell_t GetArrayValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
		return BAD_HANDLE;
	}

	int index = params[2];

	json_t *value = json_array_get(object, index);
	if (value == NULL)
	{
		pContext->ThrowNativeError("Could not retrieve value at index %d", index);
		return BAD_HANDLE;
	}

	Handle_t hndlValue = handlesys->CreateHandleEx(htJSONObject, value, &sec, NULL, NULL);
	if (hndlValue == BAD_HANDLE)
	{
		pContext->ThrowNativeError("Could not create value handle.");
		return BAD_HANDLE;
	}

	// Increase the reference counter, meaning the value handle must be
	// freed via delete or CloseHandle().
	json_incref(value);

	return hndlValue;
}

static cell_t GetArrayBoolValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	int index = params[2];

	json_t *value = json_array_get(object, index);
	if (value == NULL)
	{
		return pContext->ThrowNativeError("Could not retrieve value at index %d", index);
	}

	return json_boolean_value(value);
}

static cell_t GetArrayFloatValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	int index = params[2];

	json_t *value = json_array_get(object, index);
	if (value == NULL)
	{
		return pContext->ThrowNativeError("Could not retrieve value at index %d", index);
	}

	return sp_ftoc(json_real_value(value));
}

static cell_t GetArrayIntValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	int index = params[2];

	json_t *value = json_array_get(object, index);
	if (value == NULL)
	{
		return pContext->ThrowNativeError("Could not retrieve value at index %d", index);
	}

	return json_integer_value(value);
}

static cell_t GetArrayStringValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	int index = params[2];

	json_t *value = json_array_get(object, index);
	if (value == NULL)
	{
		return pContext->ThrowNativeError("Could not retrieve value at index %d", index);
	}

	const char *result = json_string_value(value);
	if (result == NULL)
	{
		return 0;
	}

	pContext->StringToLocalUTF8(params[3], params[4], result, NULL);

	return 1;
}

static cell_t IsArrayNullValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	int index = params[2];

	json_t *value = json_array_get(object, index);
	if (value == NULL)
	{
		return pContext->ThrowNativeError("Could not retrieve value at index %d", index);
	}

	return json_is_null(value);
}

static cell_t SetArrayValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	int index = params[2];

	json_t *value;
	Handle_t hndlValue = static_cast<Handle_t>(params[3]);
	if ((err=handlesys->ReadHandle(hndlValue, htJSONObject, &sec, (void **)&value)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid value handle %x (error %d)", hndlValue, err);
	}

	return (json_array_set(object, index, value) == 0);
}

static cell_t SetArrayBoolValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	int index = params[2];

	json_t *value = json_boolean(params[3]);

	return (json_array_set_new(object, index, value) == 0);
}

static cell_t SetArrayFloatValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	int index = params[2];

	json_t *value = json_real(sp_ctof(params[3]));

	return (json_array_set_new(object, index, value) == 0);
}

static cell_t SetArrayIntValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	int index = params[2];

	json_t *value = json_integer(params[3]);

	return (json_array_set_new(object, index, value) == 0);
}

static cell_t SetArrayNullValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	int index = params[2];

	json_t *value = json_null();

	return (json_array_set_new(object, index, value) == 0);
}

static cell_t SetArrayStringValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	int index = params[2];

	char *val;
	pContext->LocalToString(params[3], &val);

	json_t *value = json_string(val);

	return (json_array_set_new(object, index, value) == 0);
}

static cell_t PushArrayValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	json_t *value;
	Handle_t hndlValue = static_cast<Handle_t>(params[2]);
	if ((err=handlesys->ReadHandle(hndlValue, htJSONObject, &sec, (void **)&value)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid value handle %x (error %d)", hndlValue, err);
	}

	return (json_array_append(object, value) == 0);
}

static cell_t PushArrayBoolValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	json_t *value = json_boolean(params[2]);

	return (json_array_append_new(object, value) == 0);
}

static cell_t PushArrayFloatValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	json_t *value = json_real(sp_ctof(params[2]));

	return (json_array_append_new(object, value) == 0);
}

static cell_t PushArrayIntValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	json_t *value = json_integer(params[2]);

	return (json_array_append_new(object, value) == 0);
}

static cell_t PushArrayNullValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	json_t *value = json_null();

	return (json_array_append_new(object, value) == 0);
}

static cell_t PushArrayStringValue(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	char *val;
	pContext->LocalToString(params[2], &val);

	json_t *value = json_string(val);

	return (json_array_append_new(object, value) == 0);
}

static cell_t RemoveFromArray(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	int index = params[2];

	return (json_array_remove(object, index) == 0);
}

static cell_t ClearArray(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid array handle %x (error %d)", hndlObject, err);
	}

	return (json_array_clear(object) == 0);
}

static cell_t FromString(IPluginContext *pContext, const cell_t *params)
{
	char *buffer;
	pContext->LocalToString(params[1], &buffer);

	json_error_t error;
	json_t *object = json_loads(buffer, 0, &error);
	if (object == NULL)
	{
		smutils->LogError(myself, "Invalid JSON in line %d, column %d: %s", error.line, error.column, error.text);
		return BAD_HANDLE;
	}

	Handle_t hndlObject = handlesys->CreateHandle(htJSONObject, object, pContext->GetIdentity(), myself->GetIdentity(), NULL);
	if (hndlObject == BAD_HANDLE)
	{
		pContext->ThrowNativeError("Could not create object handle.");
		return BAD_HANDLE;
	}

	return hndlObject;
}

static cell_t FromFile(IPluginContext *pContext, const cell_t *params)
{
	char *file;
	pContext->LocalToString(params[1], &file);

	json_error_t error;
	json_t *object = json_load_file(file, 0, &error);
	if (object == NULL)
	{
		smutils->LogError(myself, "Invalid JSON in line %d, column %d: %s", error.line, error.column, error.text);
		return BAD_HANDLE;
	}

	Handle_t hndlObject = handlesys->CreateHandle(htJSONObject, object, pContext->GetIdentity(), myself->GetIdentity(), NULL);
	if (hndlObject == BAD_HANDLE)
	{
		pContext->ThrowNativeError("Could not create object handle.");
		return BAD_HANDLE;
	}

	return hndlObject;
}

static cell_t ToString(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	size_t flags = (size_t)params[4];

	char *result = json_dumps(object, flags);
	if (result == NULL)
	{
		return 0;
	}

	pContext->StringToLocalUTF8(params[2], params[3], result, NULL);
	free(result);

	return 1;
}

static cell_t ToFile(IPluginContext *pContext, const cell_t *params)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	json_t *object;
	Handle_t hndlObject = static_cast<Handle_t>(params[1]);
	if ((err=handlesys->ReadHandle(hndlObject, htJSONObject, &sec, (void **)&object)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid object handle %x (error %d)", hndlObject, err);
	}

	char *file;
	pContext->LocalToString(params[2], &file);

	size_t flags = (size_t)params[3];

	return (json_dump_file(object, file, flags) == 0);
}


const sp_nativeinfo_t json_natives[] =
{
	// Objects
	{"JSONObject.JSONObject",			CreateObject},
	{"JSONObject.Size.get",				GetObjectSize},
	{"JSONObject.Get",					GetObjectValue},
	{"JSONObject.GetBool",				GetObjectBoolValue},
	{"JSONObject.GetFloat",				GetObjectFloatValue},
	{"JSONObject.GetInt",				GetObjectIntValue},
	{"JSONObject.GetString",			GetObjectStringValue},
	{"JSONObject.IsNull",				IsObjectNullValue},
	{"JSONObject.Set",					SetObjectValue},
	{"JSONObject.SetBool",				SetObjectBoolValue},
	{"JSONObject.SetFloat",				SetObjectFloatValue},
	{"JSONObject.SetInt",				SetObjectIntValue},
	{"JSONObject.SetNull",				SetObjectNullValue},
	{"JSONObject.SetString",			SetObjectStringValue},
	{"JSONObject.Remove",				RemoveFromObject},
	{"JSONObject.Clear",				ClearObject},

	// Arrays
	{"JSONArray.JSONArray",				CreateArray},
	{"JSONArray.Length.get",			GetArraySize},
	{"JSONArray.Get",					GetArrayValue},
	{"JSONArray.GetBool",				GetArrayBoolValue},
	{"JSONArray.GetFloat",				GetArrayFloatValue},
	{"JSONArray.GetInt",				GetArrayIntValue},
	{"JSONArray.GetString",				GetArrayStringValue},
	{"JSONArray.IsNull",				IsArrayNullValue},
	{"JSONArray.Set",					SetArrayValue},
	{"JSONArray.SetBool",				SetArrayBoolValue},
	{"JSONArray.SetFloat",				SetArrayFloatValue},
	{"JSONArray.SetInt",				SetArrayIntValue},
	{"JSONArray.SetNull",				SetArrayNullValue},
	{"JSONArray.SetString",				SetArrayStringValue},
	{"JSONArray.Push",					PushArrayValue},
	{"JSONArray.PushBool",				PushArrayBoolValue},
	{"JSONArray.PushFloat",				PushArrayFloatValue},
	{"JSONArray.PushInt",				PushArrayIntValue},
	{"JSONArray.PushNull",				PushArrayNullValue},
	{"JSONArray.PushString",			PushArrayStringValue},
	{"JSONArray.Remove",				RemoveFromArray},
	{"JSONArray.Clear",					ClearArray},

	// Decoding
	{"JSONObject.FromString",			FromString},
	{"JSONObject.FromFile",				FromFile},
	{"JSONArray.FromString",			FromString},
	{"JSONArray.FromFile",				FromFile},

	// Encoding
	{"JSON.ToString",					ToString},
	{"JSON.ToFile",						ToFile},

	{NULL,								NULL}
};
