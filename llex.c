/*
** $Id: llex.c $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ llex_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <locale.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lctype.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lgc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llex.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lparser.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltable.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lzio.h"



#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਗਲਾ(ls)	(ls->current = zgetc(ls->z))


/* minimum size for string buffer */
#ਜੇ !defined(LUA_MINBUFFER)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_MINBUFFER   32
#ਜੇ_ਅੰਤ


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


/* ORDER RESERVED */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ lua_X_ਟੋਕਨ [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "global", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ save_and_ਅਗਲਾ(ls) (save(ls, ls->current), ਅਗਲਾ(ls))


ਸਥਿਰ l_noret lexerror (LexState *ls, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ, ਪੂਰਨ token);


ਸਥਿਰ ਖਾਲੀ save (LexState *ls, ਪੂਰਨ c) {
  Mbuffer *b = ls->buff;
  ਜੇ (luaZ_bufflen(b) + 1 > luaZ_sizebuffer(b)) {
    size_t newsize = luaZ_sizebuffer(b);  /* get old size */;
    ਜੇ (newsize >= (MAX_ਮਾਪ/3 * 2))  /* larger than MAX_SIZE/1.5 ? */
      lexerror(ls, "lexical element too long", 0);
    newsize += (newsize >> 1);  /* new size is 1.5 times the old one */
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->ਬਫਰ[luaZ_bufflen(b)++] = cast_char(c);
}


ਖਾਲੀ luaX_init (lua_State *L) {
  ਪੂਰਨ i;
  TString *e = luaS_newliteral(L, LUA_ENV);  /* create env name */
  luaC_fix(L, obj2gco(e));  /* never collect this name */
  ਲਈ (i=0; i<ਅੰਕ_RESERVED; i++) {
    TString *ts = luaS_new(L, lua_X_ਟੋਕਨ[i]);
    luaC_fix(L, obj2gco(ts));  /* reserved words are never collected */
    ts->extra = cast_byte(i+1);  /* reserved word */
  }
}


ਅਚਲ ਅੱਖਰ *luaX_token2str (LexState *ls, ਪੂਰਨ token) {
  ਜੇ (token < ਪਹਿਲਾ_RESERVED) {  /* single-byte symbols? */
    ਜੇ (lisprint(token))
      ਵਾਪਸ luaO_pushfstring(ls->L, "'%c'", token);
    ਨਹੀਂਤਾਂ  /* control character */
      ਵਾਪਸ luaO_pushfstring(ls->L, "'<\\%d>'", token);
  }
  ਨਹੀਂਤਾਂ {
    ਅਚਲ ਅੱਖਰ *s = lua_X_ਟੋਕਨ[token - ਪਹਿਲਾ_RESERVED];
    ਜੇ (token < TK_EOS)  /* fixed format (symbols and reserved words)? */
      ਵਾਪਸ luaO_pushfstring(ls->L, "'%s'", s);
    ਨਹੀਂਤਾਂ  /* names, strings, and numerals */
      ਵਾਪਸ s;
  }
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *txtToken (LexState *ls, ਪੂਰਨ token) {
  ਚੋਣ (token) {
    ਹਾਲਤ TK_NAME: ਹਾਲਤ TK_STRING:
    ਹਾਲਤ TK_FLT: ਹਾਲਤ TK_INT:
      save(ls, '\0');
      ਵਾਪਸ luaO_pushfstring(ls->L, "'%s'", lua_Z_ਬਫਰ(ls->buff));
    ਮੂਲ:
      ਵਾਪਸ luaX_token2str(ls, token);
  }
}


ਸਥਿਰ l_noret lexerror (LexState *ls, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ, ਪੂਰਨ token) {
  ਸੁਨੇਹਾ = luaG_addinfo(ls->L, ਸੁਨੇਹਾ, ls->ਸਰੋਤ, ls->linenumber);
  ਜੇ (token)
    luaO_pushfstring(ls->L, "%s near %s", ਸੁਨੇਹਾ, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


l_noret luaX_syntaxerror (LexState *ls, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ) {
  lexerror(ls, ਸੁਨੇਹਾ, ls->t.token);
}


/*
** Anchors a string in scanner's table so that it will not be collected
** until the end of the compilation; by that time it should be anchored
** somewhere. It also internalizes long strings, ensuring there is only
** one copy of each unique string.
*/
ਸਥਿਰ TString *anchorstr (LexState *ls, TString *ts) {
  lua_State *L = ls->L;
  TValue oldts;
  ਪੂਰਨ ਟੈਗ = luaH_getstr(ls->h, ts, &oldts);
  ਜੇ (!tagisempty(ਟੈਗ))  /* string already present? */
    ਵਾਪਸ tsvalue(&oldts);  /* use stored value */
  ਨਹੀਂਤਾਂ {  /* create a new entry */
    TValue *stv = s2v(L->ਸਿਖਰ.p++);  /* reserve stack space for string */
    setsvalue(L, stv, ts);  /* push (anchor) the string on the stack */
    lua_H_ਸੈੱਟ(L, ls->h, stv, stv);  /* t[string] = string */
    /* table is not a metatable, so it does not need to invalidate cache */
    luaC_checkGC(L);
    L->ਸਿਖਰ.p--;  /* remove string from stack */
    ਵਾਪਸ ts;
  }
}


/*
** Creates a new string and anchors it in scanner's table.
*/
TString *luaX_newstring (LexState *ls, ਅਚਲ ਅੱਖਰ *str, size_t l) {
  ਵਾਪਸ anchorstr(ls, luaS_newlstr(ls->L, str, l));
}


/*
** increment line number and skips newline sequence (any of
** \n, \r, \n\r, or \r\n)
*/
ਸਥਿਰ ਖਾਲੀ inclinenumber (LexState *ls) {
  ਪੂਰਨ old = ls->current;
  lua_assert(currIsNewline(ls));
  ਅਗਲਾ(ls);  /* skip '\n' or '\r' */
  ਜੇ (currIsNewline(ls) && ls->current != old)
    ਅਗਲਾ(ls);  /* skip '\n\r' or '\r\n' */
  ਜੇ (++ls->linenumber >= ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ)
    lexerror(ls, "chunk has too many lines", 0);
}


ਖਾਲੀ luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *ਸਰੋਤ,
                    ਪੂਰਨ firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = ਨਿਰਾ;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->ਸਰੋਤ = ਸਰੋਤ;
  /* all three strings here ("_ENV", "break", "global") were fixed,
     so they cannot be collected */
  ls->envn = luaS_newliteral(L, LUA_ENV);  /* get env string */
  ls->brkn = luaS_newliteral(L, "break");  /* get "break" string */
#ਜੇ LUA_COMPAT_GLOBAL
  /* compatibility mode: "global" is not a reserved word */
  ls->glbn = luaS_newliteral(L, "global");  /* get "global" string */
  ls->glbn->extra = 0;  /* mark it as not reserved */
#ਜੇ_ਅੰਤ
  luaZ_resizebuffer(ls->L, ls->buff, LUA_MINBUFFER);  /* initialize buffer */
}



/*
** =======================================================
** LEXICAL ANALYZER
** =======================================================
*/


ਸਥਿਰ ਪੂਰਨ check_next1 (LexState *ls, ਪੂਰਨ c) {
  ਜੇ (ls->current == c) {
    ਅਗਲਾ(ls);
    ਵਾਪਸ 1;
  }
  ਨਹੀਂਤਾਂ ਵਾਪਸ 0;
}


/*
** Check whether current char is in set 'set' (with two chars) and
** saves it
*/
ਸਥਿਰ ਪੂਰਨ check_next2 (LexState *ls, ਅਚਲ ਅੱਖਰ *ਸੈੱਟ) {
  lua_assert(ਸੈੱਟ[2] == '\0');
  ਜੇ (ls->current == ਸੈੱਟ[0] || ls->current == ਸੈੱਟ[1]) {
    save_and_ਅਗਲਾ(ls);
    ਵਾਪਸ 1;
  }
  ਨਹੀਂਤਾਂ ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ read_numeral (LexState *ls, SemInfo *seminfo) {
  TValue obj;
  ਅਚਲ ਅੱਖਰ *expo = "Ee";
  ਪੂਰਨ ਪਹਿਲਾ = ls->current;
  lua_assert(lisdigit(ls->current));
  save_and_ਅਗਲਾ(ls);
  ਜੇ (ਪਹਿਲਾ == '0' && check_next2(ls, "xX"))  /* hexadecimal? */
    expo = "Pp";
  ਲਈ (;;) {
    ਜੇ (check_next2(ls, expo))  /* exponent mark? */
      check_next2(ls, "-+");  /* optional exponent sign */
    ਨਹੀਂਤਾਂ ਜੇ (lisxdigit(ls->current) || ls->current == '.')  /* '%x|%.' */
      save_and_ਅਗਲਾ(ls);
    ਨਹੀਂਤਾਂ ਰੋਕੋ;
  }
  ਜੇ (lislalpha(ls->current))  /* is numeral touching a letter? */
    save_and_ਅਗਲਾ(ls);  /* force an error */
  save(ls, '\0');
  ਜੇ (luaO_str2num(lua_Z_ਬਫਰ(ls->buff), &obj) == 0)  /* format error? */
    lexerror(ls, "malformed number", TK_FLT);
  ਜੇ (ttisinteger(&obj)) {
    seminfo->i = ivalue(&obj);
    ਵਾਪਸ TK_INT;
  }
  ਨਹੀਂਤਾਂ {
    lua_assert(ttisfloat(&obj));
    seminfo->r = fltvalue(&obj);
    ਵਾਪਸ TK_FLT;
  }
}


/*
** read a sequence '[=*[' or ']=*]', leaving the last bracket. If
** sequence is well formed, return its number of '='s + 2; otherwise,
** return 1 if it is a single bracket (no '='s and no 2nd bracket);
** otherwise (an unfinished '[==...') return 0.
*/
ਸਥਿਰ size_t skip_sep (LexState *ls) {
  size_t ਗਿਣਤੀ_2 = 0;
  ਪੂਰਨ s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_ਅਗਲਾ(ls);
  ਜਦੋਂ (ls->current == '=') {
    save_and_ਅਗਲਾ(ls);
    ਗਿਣਤੀ_2++;
  }
  ਵਾਪਸ (ls->current == s) ? ਗਿਣਤੀ_2 + 2
         : (ਗਿਣਤੀ_2 == 0) ? 1
         : 0;
}


ਸਥਿਰ ਖਾਲੀ read_long_string (LexState *ls, SemInfo *seminfo, size_t sep) {
  ਪੂਰਨ ਸਤਰ = ls->linenumber;  /* initial line (for error message) */
  save_and_ਅਗਲਾ(ls);  /* skip 2nd '[' */
  ਜੇ (currIsNewline(ls))  /* string starts with a newline? */
    inclinenumber(ls);  /* skip it */
  ਲਈ (;;) {
    ਚੋਣ (ls->current) {
      ਹਾਲਤ EOZ: {  /* error */
        ਅਚਲ ਅੱਖਰ *what = (seminfo ? "string" : "comment");
        ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, ਸਤਰ);
        lexerror(ls, ਸੁਨੇਹਾ, TK_EOS);
        ਰੋਕੋ;  /* to avoid warnings */
      }
      ਹਾਲਤ ']': {
        ਜੇ (skip_sep(ls) == sep) {
          save_and_ਅਗਲਾ(ls);  /* skip 2nd ']' */
          ਜਾਓ endloop;
        }
        ਰੋਕੋ;
      }
      ਹਾਲਤ '\n': ਹਾਲਤ '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        ਜੇ (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        ਰੋਕੋ;
      }
      ਮੂਲ: {
        ਜੇ (seminfo) save_and_ਅਗਲਾ(ls);
        ਨਹੀਂਤਾਂ ਅਗਲਾ(ls);
      }
    }
  } endloop:
  ਜੇ (seminfo)
    seminfo->ts = luaX_newstring(ls, lua_Z_ਬਫਰ(ls->buff) + sep,
                                     luaZ_bufflen(ls->buff) - 2 * sep);
}


ਸਥਿਰ ਖਾਲੀ esccheck (LexState *ls, ਪੂਰਨ c, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ) {
  ਜੇ (!c) {
    ਜੇ (ls->current != EOZ)
      save_and_ਅਗਲਾ(ls);  /* add current to buffer for error message */
    lexerror(ls, ਸੁਨੇਹਾ, TK_STRING);
  }
}


ਸਥਿਰ ਪੂਰਨ gethexa (LexState *ls) {
  save_and_ਅਗਲਾ(ls);
  esccheck (ls, lisxdigit(ls->current), "hexadecimal digit expected");
  ਵਾਪਸ luaO_hexavalue(ls->current);
}


ਸਥਿਰ ਪੂਰਨ readhexaesc (LexState *ls) {
  ਪੂਰਨ r = gethexa(ls);
  r = (r << 4) + gethexa(ls);
  luaZ_buffremove(ls->buff, 2);  /* remove saved chars from buffer */
  ਵਾਪਸ r;
}


/*
** When reading a UTF-8 escape sequence, save everything to the buffer
** for error reporting in case of errors; 'i' counts the number of
** saved characters, so that they can be removed if case of success.
*/
ਸਥਿਰ l_uint32 readutf8esc (LexState *ls) {
  l_uint32 r;
  ਪੂਰਨ i = 4;  /* number of chars to be removed: start with #"\u{X" */
  save_and_ਅਗਲਾ(ls);  /* skip 'u' */
  esccheck(ls, ls->current == '{', "missing '{'");
  r = cast_uint(gethexa(ls));  /* must have at least one digit */
  ਜਦੋਂ (cast_void(save_and_ਅਗਲਾ(ls)), lisxdigit(ls->current)) {
    i++;
    esccheck(ls, r <= (0x7FFFFFFFu >> 4), "UTF-8 value too large");
    r = (r << 4) + luaO_hexavalue(ls->current);
  }
  esccheck(ls, ls->current == '}', "missing '}'");
  ਅਗਲਾ(ls);  /* skip '}' */
  luaZ_buffremove(ls->buff, i);  /* remove saved chars from buffer */
  ਵਾਪਸ r;
}


ਸਥਿਰ ਖਾਲੀ utf8esc (LexState *ls) {
  ਅੱਖਰ buff[UTF8BUFFSZ];
  ਪੂਰਨ n = luaO_utf8esc(buff, readutf8esc(ls));
  ਲਈ (; n > 0; n--)  /* add 'buff' to string */
    save(ls, buff[UTF8BUFFSZ - n]);
}


ਸਥਿਰ ਪੂਰਨ readdecesc (LexState *ls) {
  ਪੂਰਨ i;
  ਪੂਰਨ r = 0;  /* result accumulator */
  ਲਈ (i = 0; i < 3 && lisdigit(ls->current); i++) {  /* read up to 3 digits */
    r = 10*r + ls->current - '0';
    save_and_ਅਗਲਾ(ls);
  }
  esccheck(ls, r <= UCHAR_MAX, "decimal escape too large");
  luaZ_buffremove(ls->buff, i);  /* remove read digits from buffer */
  ਵਾਪਸ r;
}


ਸਥਿਰ ਖਾਲੀ read_string (LexState *ls, ਪੂਰਨ del, SemInfo *seminfo) {
  save_and_ਅਗਲਾ(ls);  /* keep delimiter (for error messages) */
  ਜਦੋਂ (ls->current != del) {
    ਚੋਣ (ls->current) {
      ਹਾਲਤ EOZ:
        lexerror(ls, "unfinished string", TK_EOS);
        ਰੋਕੋ;  /* to avoid warnings */
      ਹਾਲਤ '\n':
      ਹਾਲਤ '\r':
        lexerror(ls, "unfinished string", TK_STRING);
        ਰੋਕੋ;  /* to avoid warnings */
      ਹਾਲਤ '\\': {  /* escape sequences */
        ਪੂਰਨ c;  /* final character to be saved */
        save_and_ਅਗਲਾ(ls);  /* keep '\\' for error messages */
        ਚੋਣ (ls->current) {
          ਹਾਲਤ 'a': c = '\a'; ਜਾਓ read_save;
          ਹਾਲਤ 'b': c = '\b'; ਜਾਓ read_save;
          ਹਾਲਤ 'f': c = '\f'; ਜਾਓ read_save;
          ਹਾਲਤ 'n': c = '\n'; ਜਾਓ read_save;
          ਹਾਲਤ 'r': c = '\r'; ਜਾਓ read_save;
          ਹਾਲਤ 't': c = '\t'; ਜਾਓ read_save;
          ਹਾਲਤ 'v': c = '\v'; ਜਾਓ read_save;
          ਹਾਲਤ 'x': c = readhexaesc(ls); ਜਾਓ read_save;
          ਹਾਲਤ 'u': utf8esc(ls);  ਜਾਓ no_save;
          ਹਾਲਤ '\n': ਹਾਲਤ '\r':
            inclinenumber(ls); c = '\n'; ਜਾਓ only_save;
          ਹਾਲਤ '\\': ਹਾਲਤ '\"': ਹਾਲਤ '\'':
            c = ls->current; ਜਾਓ read_save;
          ਹਾਲਤ EOZ: ਜਾਓ no_save;  /* will raise an error next loop */
          ਹਾਲਤ 'z': {  /* zap following span of spaces */
            luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
            ਅਗਲਾ(ls);  /* skip the 'z' */
            ਜਦੋਂ (lisspace(ls->current)) {
              ਜੇ (currIsNewline(ls)) inclinenumber(ls);
              ਨਹੀਂਤਾਂ ਅਗਲਾ(ls);
            }
            ਜਾਓ no_save;
          }
          ਮੂਲ: {
            esccheck(ls, lisdigit(ls->current), "invalid escape sequence");
            c = readdecesc(ls);  /* digital escape '\ddd' */
            ਜਾਓ only_save;
          }
        }
       read_save:
         ਅਗਲਾ(ls);
         /* go through */
       only_save:
         luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
         save(ls, c);
         /* go through */
       no_save: ਰੋਕੋ;
      }
      ਮੂਲ:
        save_and_ਅਗਲਾ(ls);
    }
  }
  save_and_ਅਗਲਾ(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, lua_Z_ਬਫਰ(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}


ਸਥਿਰ ਪੂਰਨ llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  ਲਈ (;;) {
    ਚੋਣ (ls->current) {
      ਹਾਲਤ '\n': ਹਾਲਤ '\r': {  /* line breaks */
        inclinenumber(ls);
        ਰੋਕੋ;
      }
      ਹਾਲਤ ' ': ਹਾਲਤ '\f': ਹਾਲਤ '\t': ਹਾਲਤ '\v': {  /* spaces */
        ਅਗਲਾ(ls);
        ਰੋਕੋ;
      }
      ਹਾਲਤ '-': {  /* '-' or '--' (comment) */
        ਅਗਲਾ(ls);
        ਜੇ (ls->current != '-') ਵਾਪਸ '-';
        /* else is a comment */
        ਅਗਲਾ(ls);
        ਜੇ (ls->current == '[') {  /* long comment? */
          size_t sep = skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* 'skip_sep' may dirty the buffer */
          ਜੇ (sep >= 2) {
            read_long_string(ls, ਨਿਰਾ, sep);  /* skip long comment */
            luaZ_resetbuffer(ls->buff);  /* previous call may dirty the buff. */
            ਰੋਕੋ;
          }
        }
        /* else short comment */
        ਜਦੋਂ (!currIsNewline(ls) && ls->current != EOZ)
          ਅਗਲਾ(ls);  /* skip until end of line (or end of file) */
        ਰੋਕੋ;
      }
      ਹਾਲਤ '[': {  /* long string or simply '[' */
        size_t sep = skip_sep(ls);
        ਜੇ (sep >= 2) {
          read_long_string(ls, seminfo, sep);
          ਵਾਪਸ TK_STRING;
        }
        ਨਹੀਂਤਾਂ ਜੇ (sep == 0)  /* '[=...' missing second bracket? */
          lexerror(ls, "invalid long string delimiter", TK_STRING);
        ਵਾਪਸ '[';
      }
      ਹਾਲਤ '=': {
        ਅਗਲਾ(ls);
        ਜੇ (check_next1(ls, '=')) ਵਾਪਸ TK_EQ;  /* '==' */
        ਨਹੀਂਤਾਂ ਵਾਪਸ '=';
      }
      ਹਾਲਤ '<': {
        ਅਗਲਾ(ls);
        ਜੇ (check_next1(ls, '=')) ਵਾਪਸ TK_LE;  /* '<=' */
        ਨਹੀਂਤਾਂ ਜੇ (check_next1(ls, '<')) ਵਾਪਸ TK_SHL;  /* '<<' */
        ਨਹੀਂਤਾਂ ਵਾਪਸ '<';
      }
      ਹਾਲਤ '>': {
        ਅਗਲਾ(ls);
        ਜੇ (check_next1(ls, '=')) ਵਾਪਸ TK_GE;  /* '>=' */
        ਨਹੀਂਤਾਂ ਜੇ (check_next1(ls, '>')) ਵਾਪਸ TK_SHR;  /* '>>' */
        ਨਹੀਂਤਾਂ ਵਾਪਸ '>';
      }
      ਹਾਲਤ '/': {
        ਅਗਲਾ(ls);
        ਜੇ (check_next1(ls, '/')) ਵਾਪਸ TK_IDIV;  /* '//' */
        ਨਹੀਂਤਾਂ ਵਾਪਸ '/';
      }
      ਹਾਲਤ '~': {
        ਅਗਲਾ(ls);
        ਜੇ (check_next1(ls, '=')) ਵਾਪਸ TK_NE;  /* '~=' */
        ਨਹੀਂਤਾਂ ਵਾਪਸ '~';
      }
      ਹਾਲਤ ':': {
        ਅਗਲਾ(ls);
        ਜੇ (check_next1(ls, ':')) ਵਾਪਸ TK_DBCOLON;  /* '::' */
        ਨਹੀਂਤਾਂ ਵਾਪਸ ':';
      }
      ਹਾਲਤ '"': ਹਾਲਤ '\'': {  /* short literal strings */
        read_string(ls, ls->current, seminfo);
        ਵਾਪਸ TK_STRING;
      }
      ਹਾਲਤ '.': {  /* '.', '..', '...', or number */
        save_and_ਅਗਲਾ(ls);
        ਜੇ (check_next1(ls, '.')) {
          ਜੇ (check_next1(ls, '.'))
            ਵਾਪਸ TK_DOTS;   /* '...' */
          ਨਹੀਂਤਾਂ ਵਾਪਸ TK_CONCAT;   /* '..' */
        }
        ਨਹੀਂਤਾਂ ਜੇ (!lisdigit(ls->current)) ਵਾਪਸ '.';
        ਨਹੀਂਤਾਂ ਵਾਪਸ read_numeral(ls, seminfo);
      }
      ਹਾਲਤ '0': ਹਾਲਤ '1': ਹਾਲਤ '2': ਹਾਲਤ '3': ਹਾਲਤ '4':
      ਹਾਲਤ '5': ਹਾਲਤ '6': ਹਾਲਤ '7': ਹਾਲਤ '8': ਹਾਲਤ '9': {
        ਵਾਪਸ read_numeral(ls, seminfo);
      }
      ਹਾਲਤ EOZ: {
        ਵਾਪਸ TK_EOS;
      }
      ਮੂਲ: {
        ਜੇ (lislalpha(ls->current)) {  /* identifier or reserved word? */
          TString *ts;
          ਕਰੋ {
            save_and_ਅਗਲਾ(ls);
          } ਜਦੋਂ (lislalnum(ls->current));
          /* find or create string */
          ts = luaS_newlstr(ls->L, lua_Z_ਬਫਰ(ls->buff),
                                   luaZ_bufflen(ls->buff));
          ਜੇ (isreserved(ts))   /* reserved word? */
            ਵਾਪਸ ts->extra - 1 + ਪਹਿਲਾ_RESERVED;
          ਨਹੀਂਤਾਂ {
            seminfo->ts = anchorstr(ls, ts);
            ਵਾਪਸ TK_NAME;
          }
        }
        ਨਹੀਂਤਾਂ {  /* single-char tokens ('+', '*', '%', '{', '}', ...) */
          ਪੂਰਨ c = ls->current;
          ਅਗਲਾ(ls);
          ਵਾਪਸ c;
        }
      }
    }
  }
}


ਖਾਲੀ lua_X_ਅਗਲਾ (LexState *ls) {
  ls->lastline = ls->linenumber;
  ਜੇ (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  ਨਹੀਂਤਾਂ
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


ਪੂਰਨ luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
  ਵਾਪਸ ls->lookahead.token;
}

