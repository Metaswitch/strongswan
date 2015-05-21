/*
 * Copyright (C) 2015 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "imv_hcd_state.h"

#include <tncif_policy.h>

#include <utils/debug.h>

typedef struct private_imv_hcd_state_t private_imv_hcd_state_t;

/**
 * Private data of an imv_hcd_state_t object.
 */
struct private_imv_hcd_state_t {

	/**
	 * Public members of imv_hcd_state_t
	 */
	imv_hcd_state_t public;

	/**
	 * TNCCS connection ID
	 */
	TNC_ConnectionID connection_id;

	/**
	 * TNCCS connection state
	 */
	TNC_ConnectionState state;

	/**
	 * Does the TNCCS connection support long message types?
	 */
	bool has_long;

	/**
	 * Does the TNCCS connection support exclusive delivery?
	 */
	bool has_excl;

	/**
	 * Maximum PA-TNC message size for this TNCCS connection
	 */
	uint32_t max_msg_len;

	/**
	 * Flags set for completed actions
	 */
	uint32_t action_flags;

	/**
	 * IMV database session associated with TNCCS connection
	 */
	imv_session_t *session;

	/**
	 * PA-TNC attribute segmentation contracts associated with TNCCS connection
	 */
	seg_contract_manager_t *contracts;

	/**
	 * IMV action recommendation
	 */
	TNC_IMV_Action_Recommendation rec;

	/**
	 * IMV evaluation result
	 */
	TNC_IMV_Evaluation_Result eval;

	/**
	 * IMV OS handshake state
	 */
	imv_hcd_handshake_state_t handshake_state;

};

METHOD(imv_state_t, get_connection_id, TNC_ConnectionID,
	private_imv_hcd_state_t *this)
{
	return this->connection_id;
}

METHOD(imv_state_t, has_long, bool,
	private_imv_hcd_state_t *this)
{
	return this->has_long;
}

METHOD(imv_state_t, has_excl, bool,
	private_imv_hcd_state_t *this)
{
	return this->has_excl;
}

METHOD(imv_state_t, set_flags, void,
	private_imv_hcd_state_t *this, bool has_long, bool has_excl)
{
	this->has_long = has_long;
	this->has_excl = has_excl;
}

METHOD(imv_state_t, set_max_msg_len, void,
	private_imv_hcd_state_t *this, uint32_t max_msg_len)
{
	this->max_msg_len = max_msg_len;
}

METHOD(imv_state_t, get_max_msg_len, uint32_t,
	private_imv_hcd_state_t *this)
{
	return this->max_msg_len;
}

METHOD(imv_state_t, set_action_flags, void,
	private_imv_hcd_state_t *this, uint32_t flags)
{
	this->action_flags |= flags;
}

METHOD(imv_state_t, get_action_flags, uint32_t,
	private_imv_hcd_state_t *this)
{
	return this->action_flags;
}

METHOD(imv_state_t, set_session, void,
	private_imv_hcd_state_t *this, imv_session_t *session)
{
	this->session = session;
}

METHOD(imv_state_t, get_session, imv_session_t*,
	private_imv_hcd_state_t *this)
{
	return this->session;
}

METHOD(imv_state_t, get_contracts, seg_contract_manager_t*,
	private_imv_hcd_state_t *this)
{
	return this->contracts;
}

METHOD(imv_state_t, get_recommendation, void,
	private_imv_hcd_state_t *this, TNC_IMV_Action_Recommendation *rec,
								  TNC_IMV_Evaluation_Result *eval)
{
	*rec = this->rec;
	*eval = this->eval;
}

METHOD(imv_state_t, set_recommendation, void,
	private_imv_hcd_state_t *this, TNC_IMV_Action_Recommendation rec,
								  TNC_IMV_Evaluation_Result eval)
{
	this->rec = rec;
	this->eval = eval;
}

METHOD(imv_state_t, update_recommendation, void,
	private_imv_hcd_state_t *this, TNC_IMV_Action_Recommendation rec,
								  TNC_IMV_Evaluation_Result eval)
{
	this->rec  = tncif_policy_update_recommendation(this->rec, rec);
	this->eval = tncif_policy_update_evaluation(this->eval, eval);
}

METHOD(imv_state_t, change_state, void,
	private_imv_hcd_state_t *this, TNC_ConnectionState new_state)
{
	this->state = new_state;
}

METHOD(imv_state_t, get_reason_string, bool,
	private_imv_hcd_state_t *this, enumerator_t *language_enumerator,
	chunk_t *reason_string, char **reason_language)
{
	return FALSE;
}

METHOD(imv_state_t, get_remediation_instructions, bool,
	private_imv_hcd_state_t *this, enumerator_t *language_enumerator,
	chunk_t *string, char **lang_code, char **uri)
{
	return FALSE;
}

METHOD(imv_state_t, destroy, void,
	private_imv_hcd_state_t *this)
{
	DESTROY_IF(this->session);
	this->contracts->destroy(this->contracts);
	free(this);
}

METHOD(imv_hcd_state_t, set_handshake_state, void,
	private_imv_hcd_state_t *this, imv_hcd_handshake_state_t new_state)
{
	this->handshake_state = new_state;
}

METHOD(imv_hcd_state_t, get_handshake_state, imv_hcd_handshake_state_t,
	private_imv_hcd_state_t *this)
{
	return this->handshake_state;
}

/**
 * Described in header.
 */
imv_state_t *imv_hcd_state_create(TNC_ConnectionID connection_id)
{
	private_imv_hcd_state_t *this;

	INIT(this,
		.public = {
			.interface = {
				.get_connection_id = _get_connection_id,
				.has_long = _has_long,
				.has_excl = _has_excl,
				.set_flags = _set_flags,
				.set_max_msg_len = _set_max_msg_len,
				.get_max_msg_len = _get_max_msg_len,
				.set_action_flags = _set_action_flags,
				.get_action_flags = _get_action_flags,
				.set_session = _set_session,
				.get_session = _get_session,
				.get_contracts = _get_contracts,
				.change_state = _change_state,
				.get_recommendation = _get_recommendation,
				.set_recommendation = _set_recommendation,
				.update_recommendation = _update_recommendation,
				.get_reason_string = _get_reason_string,
				.get_remediation_instructions = _get_remediation_instructions,
				.destroy = _destroy,
			},
			.set_handshake_state = _set_handshake_state,
			.get_handshake_state = _get_handshake_state,
		},
		.state = TNC_CONNECTION_STATE_CREATE,
		.rec = TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION,
		.eval = TNC_IMV_EVALUATION_RESULT_DONT_KNOW,
		.connection_id = connection_id,
		.contracts = seg_contract_manager_create(),
	);

	return &this->public.interface;
}


