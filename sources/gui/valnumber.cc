//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// wxValidator specialization to gbtNumber
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#if defined(__GNUG__) && !defined(__APPLE_CC__)
#pragma implementation "valnumber.h"
#endif  // __GNUG__

#include "valnumber.h"
#include "base/base.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

BEGIN_EVENT_TABLE(gbtNumberValidator, wxValidator)
  EVT_CHAR(gbtNumberValidator::OnChar)
END_EVENT_TABLE()

static bool IsNumeric(const wxString &p_value)
{
  bool seenDigit = false, seenSeparator = false;

  for (int i = 0; i < (int) p_value.Length(); i++) {
    if (isdigit(p_value[i])) {
      seenDigit = true;
      continue;
    }
    else if ((p_value[i] == '.' && !seenSeparator) ||
	     (p_value[i] == '/' && !seenSeparator && seenDigit)) {
      seenSeparator = true;
      continue;
    }
    else if (i == 0 && p_value[i] == '-') {
      continue;
    }
    return false;
  }

  if (p_value[p_value.Length() - 1] == '/') {
    return false;
  }
  else {
    return true;
  }
}

//------------------------------------------------------------------------
//               class gbtNumberValidator: Member functions
//------------------------------------------------------------------------

gbtNumberValidator::gbtNumberValidator(wxString *p_value)
  : m_stringValue(p_value), m_hasMin(false), m_hasMax(false)
{ }

gbtNumberValidator::gbtNumberValidator(wxString *p_value,
				   const gbtNumber &p_minValue)
  : m_stringValue(p_value), m_hasMin(true), m_hasMax(false),
    m_minValue(p_minValue)
{ }

gbtNumberValidator::gbtNumberValidator(wxString *p_value,
				   const gbtNumber &p_minValue,
				   const gbtNumber &p_maxValue)
  : m_stringValue(p_value), m_hasMin(true), m_hasMax(true),
    m_minValue(p_minValue), m_maxValue(p_maxValue)
{ }

gbtNumberValidator::gbtNumberValidator(const gbtNumberValidator &p_validator)
{
  Copy(p_validator);
}

bool gbtNumberValidator::Copy(const gbtNumberValidator &p_validator)
{
  wxValidator::Copy(p_validator);
  m_stringValue = p_validator.m_stringValue;
  m_hasMin = p_validator.m_hasMin;
  m_hasMax = p_validator.m_hasMax;
  m_minValue = p_validator.m_minValue;
  m_maxValue = p_validator.m_maxValue;
  return true;
}

bool gbtNumberValidator::Validate(wxWindow *p_parent)
{
  if (!m_stringValue) {
    return false;
  }

  wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;

  if (!control->IsEnabled()) {
    return true;
  }

  wxString value(control->GetValue());

  if (!IsNumeric(value)) {
    wxMessageBox(wxT("The value ") + value + wxT(" in ") +
		 m_validatorWindow->GetName() + wxT(" is not a valid number."),
		 _("Error"), wxOK | wxICON_EXCLAMATION, p_parent);
    m_validatorWindow->SetFocus();
    return false;
  }

  if ((m_hasMin && ToNumber(gbtText(value.mb_str())) < m_minValue) ||
      (m_hasMax && ToNumber(gbtText(value.mb_str())) > m_maxValue)) {
    wxMessageBox(wxT("The value ") + value + wxT(" in ") +
		 m_validatorWindow->GetName() + wxT(" is out of the range [") +
		 wxString::Format(wxT("%s"), (char *) ToText(m_minValue)) + 
		 wxT(", ") + 
		 wxString::Format(wxT("%s"), (char *) ToText(m_maxValue)) +
		 wxT("]."),
		 _("Error"), wxOK | wxICON_EXCLAMATION, p_parent);
    m_validatorWindow->SetFocus();
    return false;
  }

  return true;
}

bool gbtNumberValidator::TransferToWindow(void)
{
  if (!m_stringValue) {
    return false;
  }

  wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
  control->SetValue(*m_stringValue);

  return true;
}

bool gbtNumberValidator::TransferFromWindow(void)
{
  if (!m_stringValue) {
    return false;
  }

  wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
  *m_stringValue = control->GetValue();

  return TRUE;
}

void gbtNumberValidator::OnChar(wxKeyEvent &p_event)
{
  if (m_validatorWindow) {
    int keyCode = (int) p_event.KeyCode();

    // we don't filter special keys and Delete
    if (!(keyCode < WXK_SPACE || keyCode == WXK_DELETE || 
	  keyCode > WXK_START) &&
	(!isdigit(keyCode) &&
	 keyCode != '.' && keyCode != '-' && keyCode != '/')) {
      if (!wxValidator::IsSilent()) {
	wxBell();
      }

      return;
    }
  }

  p_event.Skip();
}
