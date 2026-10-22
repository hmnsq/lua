/*
** $Id: llex.c $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು llex_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <locale.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lctype.h"
#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lgc.h"
#ಅನ್ತರ್ಭಾವ "llex.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lparser.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lstring.h"
#ಅನ್ತರ್ಭಾವ "ltable.h"
#ಅನ್ತರ್ಭಾವ "lzio.h"



#ವ್ಯಾಖ್ಯಾನಿಸು ಮುಂದಿನ(ls)	(ls->current = zgetc(ls->z))


/* minimum size for string buffer */
#ಆದರೆ !defined(LUA_MINBUFFER)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MINBUFFER   32
#ಆದರೆ_ಅಂತ್ಯ


#ವ್ಯಾಖ್ಯಾನಿಸು currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


/* ORDER RESERVED */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ lua_X_ಟೋಕನುಗಳು [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "global", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};


#ವ್ಯಾಖ್ಯಾನಿಸು save_and_ಮುಂದಿನ(ls) (save(ls, ls->current), ಮುಂದಿನ(ls))


ಸ್ಥಿರ l_noret lexerror (LexState *ls, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ, ಪೂರ್ಣಾಂಕ token);


ಸ್ಥಿರ ಶೂನ್ಯ save (LexState *ls, ಪೂರ್ಣಾಂಕ c) {
  Mbuffer *b = ls->buff;
  ಆದರೆ (luaZ_bufflen(b) + 1 > luaZ_sizebuffer(b)) {
    size_t newsize = luaZ_sizebuffer(b);  /* get old size */;
    ಆದರೆ (newsize >= (MAX_ಗಾತ್ರ/3 * 2))  /* larger than MAX_SIZE/1.5 ? */
      lexerror(ls, "lexical element too long", 0);
    newsize += (newsize >> 1);  /* new size is 1.5 times the old one */
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->ಬಫರ್[luaZ_bufflen(b)++] = cast_char(c);
}


ಶೂನ್ಯ luaX_init (lua_State *L) {
  ಪೂರ್ಣಾಂಕ i;
  TString *e = luaS_newliteral(L, LUA_ENV);  /* create env name */
  luaC_fix(L, obj2gco(e));  /* never collect this name */
  ಗಾಗಿ (i=0; i<ಅಂಕೆ_RESERVED; i++) {
    TString *ts = luaS_new(L, lua_X_ಟೋಕನುಗಳು[i]);
    luaC_fix(L, obj2gco(ts));  /* reserved words are never collected */
    ts->extra = cast_byte(i+1);  /* reserved word */
  }
}


ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaX_token2str (LexState *ls, ಪೂರ್ಣಾಂಕ token) {
  ಆದರೆ (token < ಮೊದಲ_RESERVED) {  /* single-byte symbols? */
    ಆದರೆ (lisprint(token))
      ಹಿಂತಿರುಗಿ luaO_pushfstring(ls->L, "'%c'", token);
    ಇಲ್ಲದಿದ್ದರೆ  /* control character */
      ಹಿಂತಿರುಗಿ luaO_pushfstring(ls->L, "'<\\%d>'", token);
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = lua_X_ಟೋಕನುಗಳು[token - ಮೊದಲ_RESERVED];
    ಆದರೆ (token < TK_EOS)  /* fixed format (symbols and reserved words)? */
      ಹಿಂತಿರುಗಿ luaO_pushfstring(ls->L, "'%s'", s);
    ಇಲ್ಲದಿದ್ದರೆ  /* names, strings, and numerals */
      ಹಿಂತಿರುಗಿ s;
  }
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *txtToken (LexState *ls, ಪೂರ್ಣಾಂಕ token) {
  ಆಯ್ಕೆ (token) {
    ಸ್ಥಿತಿ TK_NAME: ಸ್ಥಿತಿ TK_STRING:
    ಸ್ಥಿತಿ TK_FLT: ಸ್ಥಿತಿ TK_INT:
      save(ls, '\0');
      ಹಿಂತಿರುಗಿ luaO_pushfstring(ls->L, "'%s'", lua_Z_ಬಫರ್(ls->buff));
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ luaX_token2str(ls, token);
  }
}


ಸ್ಥಿರ l_noret lexerror (LexState *ls, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ, ಪೂರ್ಣಾಂಕ token) {
  ಸಂದೇಶ = luaG_addinfo(ls->L, ಸಂದೇಶ, ls->ಮೂಲ, ls->linenumber);
  ಆದರೆ (token)
    luaO_pushfstring(ls->L, "%s near %s", ಸಂದೇಶ, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


l_noret luaX_syntaxerror (LexState *ls, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ) {
  lexerror(ls, ಸಂದೇಶ, ls->t.token);
}


/*
** Anchors a string in scanner's table so that it will not be collected
** until the end of the compilation; by that time it should be anchored
** somewhere. It also internalizes long strings, ensuring there is only
** one copy of each unique string.
*/
ಸ್ಥಿರ TString *anchorstr (LexState *ls, TString *ts) {
  lua_State *L = ls->L;
  TValue oldts;
  ಪೂರ್ಣಾಂಕ ಟ್ಯಾಗ್ = luaH_getstr(ls->h, ts, &oldts);
  ಆದರೆ (!tagisempty(ಟ್ಯಾಗ್))  /* string already present? */
    ಹಿಂತಿರುಗಿ tsvalue(&oldts);  /* use stored value */
  ಇಲ್ಲದಿದ್ದರೆ {  /* create a new entry */
    TValue *stv = s2v(L->ತುದಿ.p++);  /* reserve stack space for string */
    setsvalue(L, stv, ts);  /* push (anchor) the string on the stack */
    lua_H_ಗಣ(L, ls->h, stv, stv);  /* t[string] = string */
    /* table is not a metatable, so it does not need to invalidate cache */
    luaC_checkGC(L);
    L->ತುದಿ.p--;  /* remove string from stack */
    ಹಿಂತಿರುಗಿ ts;
  }
}


/*
** Creates a new string and anchors it in scanner's table.
*/
TString *luaX_newstring (LexState *ls, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, size_t l) {
  ಹಿಂತಿರುಗಿ anchorstr(ls, luaS_newlstr(ls->L, str, l));
}


/*
** increment line number and skips newline sequence (any of
** \n, \r, \n\r, or \r\n)
*/
ಸ್ಥಿರ ಶೂನ್ಯ inclinenumber (LexState *ls) {
  ಪೂರ್ಣಾಂಕ old = ls->current;
  lua_assert(currIsNewline(ls));
  ಮುಂದಿನ(ls);  /* skip '\n' or '\r' */
  ಆದರೆ (currIsNewline(ls) && ls->current != old)
    ಮುಂದಿನ(ls);  /* skip '\n\r' or '\r\n' */
  ಆದರೆ (++ls->linenumber >= ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ)
    lexerror(ls, "chunk has too many lines", 0);
}


ಶೂನ್ಯ luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *ಮೂಲ,
                    ಪೂರ್ಣಾಂಕ firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = ಖಾಲಿ;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->ಮೂಲ = ಮೂಲ;
  /* all three strings here ("_ENV", "break", "global") were fixed,
     so they cannot be collected */
  ls->envn = luaS_newliteral(L, LUA_ENV);  /* get env string */
  ls->brkn = luaS_newliteral(L, "break");  /* get "break" string */
#ಆದರೆ LUA_COMPAT_GLOBAL
  /* compatibility mode: "global" is not a reserved word */
  ls->glbn = luaS_newliteral(L, "global");  /* get "global" string */
  ls->glbn->extra = 0;  /* mark it as not reserved */
#ಆದರೆ_ಅಂತ್ಯ
  luaZ_resizebuffer(ls->L, ls->buff, LUA_MINBUFFER);  /* initialize buffer */
}



/*
** =======================================================
** LEXICAL ANALYZER
** =======================================================
*/


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_next1 (LexState *ls, ಪೂರ್ಣಾಂಕ c) {
  ಆದರೆ (ls->current == c) {
    ಮುಂದಿನ(ls);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ 0;
}


/*
** Check whether current char is in set 'set' (with two chars) and
** saves it
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_next2 (LexState *ls, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗಣ) {
  lua_assert(ಗಣ[2] == '\0');
  ಆದರೆ (ls->current == ಗಣ[0] || ls->current == ಗಣ[1]) {
    save_and_ಮುಂದಿನ(ls);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ 0;
}


/* LUA_NUMBER */
/*
** This function is quite liberal in what it accepts, as 'luaO_str2num'
** will reject ill-formed numerals. Roughly, it accepts the following
** pattern:
**
**   %d(%x|%.|([Ee][+-]?))* | 0[Xx](%x|%.|([Pp][+-]?))*
**
** The only tricky part is to accept [+-] only after a valid exponent
** mark, to avoid reading '3-4' or '0xe+1' as a single number.
**
** The caller might have already read an initial dot.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ read_numeral (LexState *ls, SemInfo *seminfo) {
  TValue obj;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *expo = "Ee";
  ಪೂರ್ಣಾಂಕ ಮೊದಲ = ls->current;
  lua_assert(lisdigit(ls->current));
  save_and_ಮುಂದಿನ(ls);
  ಆದರೆ (ಮೊದಲ == '0' && check_next2(ls, "xX"))  /* hexadecimal? */
    expo = "Pp";
  ಗಾಗಿ (;;) {
    ಆದರೆ (check_next2(ls, expo))  /* exponent mark? */
      check_next2(ls, "-+");  /* optional exponent sign */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (lisxdigit(ls->current) || ls->current == '.')  /* '%x|%.' */
      save_and_ಮುಂದಿನ(ls);
    ಇಲ್ಲದಿದ್ದರೆ ನಿಲ್ಲಿಸು;
  }
  ಆದರೆ (lislalpha(ls->current))  /* is numeral touching a letter? */
    save_and_ಮುಂದಿನ(ls);  /* force an error */
  save(ls, '\0');
  ಆದರೆ (luaO_str2num(lua_Z_ಬಫರ್(ls->buff), &obj) == 0)  /* format error? */
    lexerror(ls, "malformed number", TK_FLT);
  ಆದರೆ (ttisinteger(&obj)) {
    seminfo->i = ivalue(&obj);
    ಹಿಂತಿರುಗಿ TK_INT;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_assert(ttisfloat(&obj));
    seminfo->r = fltvalue(&obj);
    ಹಿಂತಿರುಗಿ TK_FLT;
  }
}


/*
** read a sequence '[=*[' or ']=*]', leaving the last bracket. If
** sequence is well formed, return its number of '='s + 2; otherwise,
** return 1 if it is a single bracket (no '='s and no 2nd bracket);
** otherwise (an unfinished '[==...') return 0.
*/
ಸ್ಥಿರ size_t skip_sep (LexState *ls) {
  size_t ಎಣಿಕೆ_2 = 0;
  ಪೂರ್ಣಾಂಕ s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_ಮುಂದಿನ(ls);
  ಆಗ (ls->current == '=') {
    save_and_ಮುಂದಿನ(ls);
    ಎಣಿಕೆ_2++;
  }
  ಹಿಂತಿರುಗಿ (ls->current == s) ? ಎಣಿಕೆ_2 + 2
         : (ಎಣಿಕೆ_2 == 0) ? 1
         : 0;
}


ಸ್ಥಿರ ಶೂನ್ಯ read_long_string (LexState *ls, SemInfo *seminfo, size_t sep) {
  ಪೂರ್ಣಾಂಕ ಸಾಲು = ls->linenumber;  /* initial line (for error message) */
  save_and_ಮುಂದಿನ(ls);  /* skip 2nd '[' */
  ಆದರೆ (currIsNewline(ls))  /* string starts with a newline? */
    inclinenumber(ls);  /* skip it */
  ಗಾಗಿ (;;) {
    ಆಯ್ಕೆ (ls->current) {
      ಸ್ಥಿತಿ EOZ: {  /* error */
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what = (seminfo ? "string" : "comment");
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, ಸಾಲು);
        lexerror(ls, ಸಂದೇಶ, TK_EOS);
        ನಿಲ್ಲಿಸು;  /* to avoid warnings */
      }
      ಸ್ಥಿತಿ ']': {
        ಆದರೆ (skip_sep(ls) == sep) {
          save_and_ಮುಂದಿನ(ls);  /* skip 2nd ']' */
          ಹೋಗು endloop;
        }
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ '\n': ಸ್ಥಿತಿ '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        ಆದರೆ (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        ನಿಲ್ಲಿಸು;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: {
        ಆದರೆ (seminfo) save_and_ಮುಂದಿನ(ls);
        ಇಲ್ಲದಿದ್ದರೆ ಮುಂದಿನ(ls);
      }
    }
  } endloop:
  ಆದರೆ (seminfo)
    seminfo->ts = luaX_newstring(ls, lua_Z_ಬಫರ್(ls->buff) + sep,
                                     luaZ_bufflen(ls->buff) - 2 * sep);
}


ಸ್ಥಿರ ಶೂನ್ಯ esccheck (LexState *ls, ಪೂರ್ಣಾಂಕ c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ) {
  ಆದರೆ (!c) {
    ಆದರೆ (ls->current != EOZ)
      save_and_ಮುಂದಿನ(ls);  /* add current to buffer for error message */
    lexerror(ls, ಸಂದೇಶ, TK_STRING);
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ gethexa (LexState *ls) {
  save_and_ಮುಂದಿನ(ls);
  esccheck (ls, lisxdigit(ls->current), "hexadecimal digit expected");
  ಹಿಂತಿರುಗಿ luaO_hexavalue(ls->current);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ readhexaesc (LexState *ls) {
  ಪೂರ್ಣಾಂಕ r = gethexa(ls);
  r = (r << 4) + gethexa(ls);
  luaZ_buffremove(ls->buff, 2);  /* remove saved chars from buffer */
  ಹಿಂತಿರುಗಿ r;
}


/*
** When reading a UTF-8 escape sequence, save everything to the buffer
** for error reporting in case of errors; 'i' counts the number of
** saved characters, so that they can be removed if case of success.
*/
ಸ್ಥಿರ l_uint32 readutf8esc (LexState *ls) {
  l_uint32 r;
  ಪೂರ್ಣಾಂಕ i = 4;  /* number of chars to be removed: start with #"\u{X" */
  save_and_ಮುಂದಿನ(ls);  /* skip 'u' */
  esccheck(ls, ls->current == '{', "missing '{'");
  r = cast_uint(gethexa(ls));  /* must have at least one digit */
  ಆಗ (cast_void(save_and_ಮುಂದಿನ(ls)), lisxdigit(ls->current)) {
    i++;
    esccheck(ls, r <= (0x7FFFFFFFu >> 4), "UTF-8 value too large");
    r = (r << 4) + luaO_hexavalue(ls->current);
  }
  esccheck(ls, ls->current == '}', "missing '}'");
  ಮುಂದಿನ(ls);  /* skip '}' */
  luaZ_buffremove(ls->buff, i);  /* remove saved chars from buffer */
  ಹಿಂತಿರುಗಿ r;
}


ಸ್ಥಿರ ಶೂನ್ಯ utf8esc (LexState *ls) {
  ಅಕ್ಷರ buff[UTF8BUFFSZ];
  ಪೂರ್ಣಾಂಕ n = luaO_utf8esc(buff, readutf8esc(ls));
  ಗಾಗಿ (; n > 0; n--)  /* add 'buff' to string */
    save(ls, buff[UTF8BUFFSZ - n]);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ readdecesc (LexState *ls) {
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ r = 0;  /* result accumulator */
  ಗಾಗಿ (i = 0; i < 3 && lisdigit(ls->current); i++) {  /* read up to 3 digits */
    r = 10*r + ls->current - '0';
    save_and_ಮುಂದಿನ(ls);
  }
  esccheck(ls, r <= UCHAR_MAX, "decimal escape too large");
  luaZ_buffremove(ls->buff, i);  /* remove read digits from buffer */
  ಹಿಂತಿರುಗಿ r;
}


ಸ್ಥಿರ ಶೂನ್ಯ read_string (LexState *ls, ಪೂರ್ಣಾಂಕ del, SemInfo *seminfo) {
  save_and_ಮುಂದಿನ(ls);  /* keep delimiter (for error messages) */
  ಆಗ (ls->current != del) {
    ಆಯ್ಕೆ (ls->current) {
      ಸ್ಥಿತಿ EOZ:
        lexerror(ls, "unfinished string", TK_EOS);
        ನಿಲ್ಲಿಸು;  /* to avoid warnings */
      ಸ್ಥಿತಿ '\n':
      ಸ್ಥಿತಿ '\r':
        lexerror(ls, "unfinished string", TK_STRING);
        ನಿಲ್ಲಿಸು;  /* to avoid warnings */
      ಸ್ಥಿತಿ '\\': {  /* escape sequences */
        ಪೂರ್ಣಾಂಕ c;  /* final character to be saved */
        save_and_ಮುಂದಿನ(ls);  /* keep '\\' for error messages */
        ಆಯ್ಕೆ (ls->current) {
          ಸ್ಥಿತಿ 'a': c = '\a'; ಹೋಗು read_save;
          ಸ್ಥಿತಿ 'b': c = '\b'; ಹೋಗು read_save;
          ಸ್ಥಿತಿ 'f': c = '\f'; ಹೋಗು read_save;
          ಸ್ಥಿತಿ 'n': c = '\n'; ಹೋಗು read_save;
          ಸ್ಥಿತಿ 'r': c = '\r'; ಹೋಗು read_save;
          ಸ್ಥಿತಿ 't': c = '\t'; ಹೋಗು read_save;
          ಸ್ಥಿತಿ 'v': c = '\v'; ಹೋಗು read_save;
          ಸ್ಥಿತಿ 'x': c = readhexaesc(ls); ಹೋಗು read_save;
          ಸ್ಥಿತಿ 'u': utf8esc(ls);  ಹೋಗು no_save;
          ಸ್ಥಿತಿ '\n': ಸ್ಥಿತಿ '\r':
            inclinenumber(ls); c = '\n'; ಹೋಗು only_save;
          ಸ್ಥಿತಿ '\\': ಸ್ಥಿತಿ '\"': ಸ್ಥಿತಿ '\'':
            c = ls->current; ಹೋಗು read_save;
          ಸ್ಥಿತಿ EOZ: ಹೋಗು no_save;  /* will raise an error next loop */
          ಸ್ಥಿತಿ 'z': {  /* zap following span of spaces */
            luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
            ಮುಂದಿನ(ls);  /* skip the 'z' */
            ಆಗ (lisspace(ls->current)) {
              ಆದರೆ (currIsNewline(ls)) inclinenumber(ls);
              ಇಲ್ಲದಿದ್ದರೆ ಮುಂದಿನ(ls);
            }
            ಹೋಗು no_save;
          }
          ಪೂರ್ವನಿಯೋಜಿತ: {
            esccheck(ls, lisdigit(ls->current), "invalid escape sequence");
            c = readdecesc(ls);  /* digital escape '\ddd' */
            ಹೋಗು only_save;
          }
        }
       read_save:
         ಮುಂದಿನ(ls);
         /* go through */
       only_save:
         luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
         save(ls, c);
         /* go through */
       no_save: ನಿಲ್ಲಿಸು;
      }
      ಪೂರ್ವನಿಯೋಜಿತ:
        save_and_ಮುಂದಿನ(ls);
    }
  }
  save_and_ಮುಂದಿನ(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, lua_Z_ಬಫರ್(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  ಗಾಗಿ (;;) {
    ಆಯ್ಕೆ (ls->current) {
      ಸ್ಥಿತಿ '\n': ಸ್ಥಿತಿ '\r': {  /* line breaks */
        inclinenumber(ls);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ ' ': ಸ್ಥಿತಿ '\f': ಸ್ಥಿತಿ '\t': ಸ್ಥಿತಿ '\v': {  /* spaces */
        ಮುಂದಿನ(ls);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ '-': {  /* '-' or '--' (comment) */
        ಮುಂದಿನ(ls);
        ಆದರೆ (ls->current != '-') ಹಿಂತಿರುಗಿ '-';
        /* else is a comment */
        ಮುಂದಿನ(ls);
        ಆದರೆ (ls->current == '[') {  /* long comment? */
          size_t sep = skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* 'skip_sep' may dirty the buffer */
          ಆದರೆ (sep >= 2) {
            read_long_string(ls, ಖಾಲಿ, sep);  /* skip long comment */
            luaZ_resetbuffer(ls->buff);  /* previous call may dirty the buff. */
            ನಿಲ್ಲಿಸು;
          }
        }
        /* else short comment */
        ಆಗ (!currIsNewline(ls) && ls->current != EOZ)
          ಮುಂದಿನ(ls);  /* skip until end of line (or end of file) */
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ '[': {  /* long string or simply '[' */
        size_t sep = skip_sep(ls);
        ಆದರೆ (sep >= 2) {
          read_long_string(ls, seminfo, sep);
          ಹಿಂತಿರುಗಿ TK_STRING;
        }
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (sep == 0)  /* '[=...' missing second bracket? */
          lexerror(ls, "invalid long string delimiter", TK_STRING);
        ಹಿಂತಿರುಗಿ '[';
      }
      ಸ್ಥಿತಿ '=': {
        ಮುಂದಿನ(ls);
        ಆದರೆ (check_next1(ls, '=')) ಹಿಂತಿರುಗಿ TK_EQ;  /* '==' */
        ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ '=';
      }
      ಸ್ಥಿತಿ '<': {
        ಮುಂದಿನ(ls);
        ಆದರೆ (check_next1(ls, '=')) ಹಿಂತಿರುಗಿ TK_LE;  /* '<=' */
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (check_next1(ls, '<')) ಹಿಂತಿರುಗಿ TK_SHL;  /* '<<' */
        ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ '<';
      }
      ಸ್ಥಿತಿ '>': {
        ಮುಂದಿನ(ls);
        ಆದರೆ (check_next1(ls, '=')) ಹಿಂತಿರುಗಿ TK_GE;  /* '>=' */
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (check_next1(ls, '>')) ಹಿಂತಿರುಗಿ TK_SHR;  /* '>>' */
        ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ '>';
      }
      ಸ್ಥಿತಿ '/': {
        ಮುಂದಿನ(ls);
        ಆದರೆ (check_next1(ls, '/')) ಹಿಂತಿರುಗಿ TK_IDIV;  /* '//' */
        ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ '/';
      }
      ಸ್ಥಿತಿ '~': {
        ಮುಂದಿನ(ls);
        ಆದರೆ (check_next1(ls, '=')) ಹಿಂತಿರುಗಿ TK_NE;  /* '~=' */
        ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ '~';
      }
      ಸ್ಥಿತಿ ':': {
        ಮುಂದಿನ(ls);
        ಆದರೆ (check_next1(ls, ':')) ಹಿಂತಿರುಗಿ TK_DBCOLON;  /* '::' */
        ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ ':';
      }
      ಸ್ಥಿತಿ '"': ಸ್ಥಿತಿ '\'': {  /* short literal strings */
        read_string(ls, ls->current, seminfo);
        ಹಿಂತಿರುಗಿ TK_STRING;
      }
      ಸ್ಥಿತಿ '.': {  /* '.', '..', '...', or number */
        save_and_ಮುಂದಿನ(ls);
        ಆದರೆ (check_next1(ls, '.')) {
          ಆದರೆ (check_next1(ls, '.'))
            ಹಿಂತಿರುಗಿ TK_DOTS;   /* '...' */
          ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ TK_CONCAT;   /* '..' */
        }
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!lisdigit(ls->current)) ಹಿಂತಿರುಗಿ '.';
        ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ read_numeral(ls, seminfo);
      }
      ಸ್ಥಿತಿ '0': ಸ್ಥಿತಿ '1': ಸ್ಥಿತಿ '2': ಸ್ಥಿತಿ '3': ಸ್ಥಿತಿ '4':
      ಸ್ಥಿತಿ '5': ಸ್ಥಿತಿ '6': ಸ್ಥಿತಿ '7': ಸ್ಥಿತಿ '8': ಸ್ಥಿತಿ '9': {
        ಹಿಂತಿರುಗಿ read_numeral(ls, seminfo);
      }
      ಸ್ಥಿತಿ EOZ: {
        ಹಿಂತಿರುಗಿ TK_EOS;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: {
        ಆದರೆ (lislalpha(ls->current)) {  /* identifier or reserved word? */
          TString *ts;
          ಮಾಡು {
            save_and_ಮುಂದಿನ(ls);
          } ಆಗ (lislalnum(ls->current));
          /* find or create string */
          ts = luaS_newlstr(ls->L, lua_Z_ಬಫರ್(ls->buff),
                                   luaZ_bufflen(ls->buff));
          ಆದರೆ (isreserved(ts))   /* reserved word? */
            ಹಿಂತಿರುಗಿ ts->extra - 1 + ಮೊದಲ_RESERVED;
          ಇಲ್ಲದಿದ್ದರೆ {
            seminfo->ts = anchorstr(ls, ts);
            ಹಿಂತಿರುಗಿ TK_NAME;
          }
        }
        ಇಲ್ಲದಿದ್ದರೆ {  /* single-char tokens ('+', '*', '%', '{', '}', ...) */
          ಪೂರ್ಣಾಂಕ c = ls->current;
          ಮುಂದಿನ(ls);
          ಹಿಂತಿರುಗಿ c;
        }
      }
    }
  }
}


ಶೂನ್ಯ lua_X_ಮುಂದಿನ (LexState *ls) {
  ls->lastline = ls->linenumber;
  ಆದರೆ (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  ಇಲ್ಲದಿದ್ದರೆ
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


ಪೂರ್ಣಾಂಕ luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
  ಹಿಂತಿರುಗಿ ls->lookahead.token;
}

