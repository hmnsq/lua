/*
** $Id: llex.c $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#define llex_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <locale.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lctype.h"
#અન્તર્ભાવ "ldebug.h"
#અન્તર્ભાવ "ldo.h"
#અન્તર્ભાવ "lgc.h"
#અન્તર્ભાવ "llex.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lparser.h"
#અન્તર્ભાવ "lstate.h"
#અન્તર્ભાવ "lstring.h"
#અન્તર્ભાવ "ltable.h"
#અન્તર્ભાવ "lzio.h"



#define next(ls)	(ls->current = zgetc(ls->z))


/* minimum size for string buffer */
#જો !defined(LUA_MINBUFFER)
#define LUA_MINBUFFER   32
#endif


#define currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


/* ORDER RESERVED */
સ્થિર અચલ અક્ષર *અચલ luaX_tokens [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "global", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};


#define save_and_next(ls) (save(ls, ls->current), next(ls))


સ્થિર l_noret lexerror (LexState *ls, અચલ અક્ષર *સંદેશ, પૂર્ણાંક token);


સ્થિર શૂન્ય save (LexState *ls, પૂર્ણાંક c) {
  Mbuffer *b = ls->buff;
  જો (luaZ_bufflen(b) + 1 > luaZ_sizebuffer(b)) {
    size_t newsize = luaZ_sizebuffer(b);  /* get old size */;
    જો (newsize >= (MAX_SIZE/3 * 2))  /* larger than MAX_SIZE/1.5 ? */
      lexerror(ls, "lexical element too long", 0);
    newsize += (newsize >> 1);  /* new size is 1.5 times the old one */
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->buffer[luaZ_bufflen(b)++] = cast_char(c);
}


શૂન્ય luaX_init (lua_State *L) {
  પૂર્ણાંક i;
  TString *e = luaS_newliteral(L, LUA_ENV);  /* create env name */
  luaC_fix(L, obj2gco(e));  /* never collect this name */
  માટે (i=0; i<અંક_RESERVED; i++) {
    TString *ts = luaS_new(L, luaX_tokens[i]);
    luaC_fix(L, obj2gco(ts));  /* reserved words are never collected */
    ts->extra = cast_byte(i+1);  /* reserved word */
  }
}


અચલ અક્ષર *luaX_token2str (LexState *ls, પૂર્ણાંક token) {
  જો (token < FIRST_RESERVED) {  /* single-byte symbols? */
    જો (lisprint(token))
      પાછા luaO_pushfstring(ls->L, "'%c'", token);
    નહીંતર  /* control character */
      પાછા luaO_pushfstring(ls->L, "'<\\%d>'", token);
  }
  નહીંતર {
    અચલ અક્ષર *s = luaX_tokens[token - FIRST_RESERVED];
    જો (token < TK_EOS)  /* fixed format (symbols and reserved words)? */
      પાછા luaO_pushfstring(ls->L, "'%s'", s);
    નહીંતર  /* names, strings, and numerals */
      પાછા s;
  }
}


સ્થિર અચલ અક્ષર *txtToken (LexState *ls, પૂર્ણાંક token) {
  પસંદગી (token) {
    સ્થિતિ TK_NAME: સ્થિતિ TK_STRING:
    સ્થિતિ TK_FLT: સ્થિતિ TK_INT:
      save(ls, '\0');
      પાછા luaO_pushfstring(ls->L, "'%s'", luaZ_buffer(ls->buff));
    મૂળભૂત:
      પાછા luaX_token2str(ls, token);
  }
}


સ્થિર l_noret lexerror (LexState *ls, અચલ અક્ષર *સંદેશ, પૂર્ણાંક token) {
  સંદેશ = luaG_addinfo(ls->L, સંદેશ, ls->source, ls->linenumber);
  જો (token)
    luaO_pushfstring(ls->L, "%s near %s", સંદેશ, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


l_noret luaX_syntaxerror (LexState *ls, અચલ અક્ષર *સંદેશ) {
  lexerror(ls, સંદેશ, ls->t.token);
}


/*
** Anchors a string in scanner's table so that it will not be collected
** until the end of the compilation; by that time it should be anchored
** somewhere. It also internalizes long strings, ensuring there is only
** one copy of each unique string.
*/
સ્થિર TString *anchorstr (LexState *ls, TString *ts) {
  lua_State *L = ls->L;
  TValue oldts;
  પૂર્ણાંક tag = luaH_getstr(ls->h, ts, &oldts);
  જો (!tagisempty(tag))  /* string already present? */
    પાછા tsvalue(&oldts);  /* use stored value */
  નહીંતર {  /* create a new entry */
    TValue *stv = s2v(L->ટોચ.p++);  /* reserve stack space for string */
    setsvalue(L, stv, ts);  /* push (anchor) the string on the stack */
    luaH_set(L, ls->h, stv, stv);  /* t[string] = string */
    /* table is not a metatable, so it does not need to invalidate cache */
    luaC_checkGC(L);
    L->ટોચ.p--;  /* remove string from stack */
    પાછા ts;
  }
}


/*
** Creates a new string and anchors it in scanner's table.
*/
TString *luaX_newstring (LexState *ls, અચલ અક્ષર *str, size_t l) {
  પાછા anchorstr(ls, luaS_newlstr(ls->L, str, l));
}


/*
** increment line number and skips newline sequence (any of
** \n, \r, \n\r, or \r\n)
*/
સ્થિર શૂન્ય inclinenumber (LexState *ls) {
  પૂર્ણાંક old = ls->current;
  lua_assert(currIsNewline(ls));
  next(ls);  /* skip '\n' or '\r' */
  જો (currIsNewline(ls) && ls->current != old)
    next(ls);  /* skip '\n\r' or '\r\n' */
  જો (++ls->linenumber >= INT_MAX)
    lexerror(ls, "chunk has too many lines", 0);
}


શૂન્ય luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *source,
                    પૂર્ણાંક firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = ખાલી;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->source = source;
  /* all three strings here ("_ENV", "break", "global") were fixed,
     so they cannot be collected */
  ls->envn = luaS_newliteral(L, LUA_ENV);  /* get env string */
  ls->brkn = luaS_newliteral(L, "break");  /* get "break" string */
#જો LUA_COMPAT_GLOBAL
  /* compatibility mode: "global" is not a reserved word */
  ls->glbn = luaS_newliteral(L, "global");  /* get "global" string */
  ls->glbn->extra = 0;  /* mark it as not reserved */
#endif
  luaZ_resizebuffer(ls->L, ls->buff, LUA_MINBUFFER);  /* initialize buffer */
}



/*
** =======================================================
** LEXICAL ANALYZER
** =======================================================
*/


સ્થિર પૂર્ણાંક check_next1 (LexState *ls, પૂર્ણાંક c) {
  જો (ls->current == c) {
    next(ls);
    પાછા 1;
  }
  નહીંતર પાછા 0;
}


/*
** Check whether current char is in set 'set' (with two chars) and
** saves it
*/
સ્થિર પૂર્ણાંક check_next2 (LexState *ls, અચલ અક્ષર *set) {
  lua_assert(set[2] == '\0');
  જો (ls->current == set[0] || ls->current == set[1]) {
    save_and_next(ls);
    પાછા 1;
  }
  નહીંતર પાછા 0;
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
સ્થિર પૂર્ણાંક read_numeral (LexState *ls, SemInfo *seminfo) {
  TValue obj;
  અચલ અક્ષર *expo = "Ee";
  પૂર્ણાંક first = ls->current;
  lua_assert(lisdigit(ls->current));
  save_and_next(ls);
  જો (first == '0' && check_next2(ls, "xX"))  /* hexadecimal? */
    expo = "Pp";
  માટે (;;) {
    જો (check_next2(ls, expo))  /* exponent mark? */
      check_next2(ls, "-+");  /* optional exponent sign */
    નહીંતર જો (lisxdigit(ls->current) || ls->current == '.')  /* '%x|%.' */
      save_and_next(ls);
    નહીંતર તોડો;
  }
  જો (lislalpha(ls->current))  /* is numeral touching a letter? */
    save_and_next(ls);  /* force an error */
  save(ls, '\0');
  જો (luaO_str2num(luaZ_buffer(ls->buff), &obj) == 0)  /* format error? */
    lexerror(ls, "malformed number", TK_FLT);
  જો (ttisinteger(&obj)) {
    seminfo->i = ivalue(&obj);
    પાછા TK_INT;
  }
  નહીંતર {
    lua_assert(ttisfloat(&obj));
    seminfo->r = fltvalue(&obj);
    પાછા TK_FLT;
  }
}


/*
** read a sequence '[=*[' or ']=*]', leaving the last bracket. If
** sequence is well formed, return its number of '='s + 2; otherwise,
** return 1 if it is a single bracket (no '='s and no 2nd bracket);
** otherwise (an unfinished '[==...') return 0.
*/
સ્થિર size_t skip_sep (LexState *ls) {
  size_t ગણતરી = 0;
  પૂર્ણાંક s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_next(ls);
  જ્યારે (ls->current == '=') {
    save_and_next(ls);
    ગણતરી++;
  }
  પાછા (ls->current == s) ? ગણતરી + 2
         : (ગણતરી == 0) ? 1
         : 0;
}


સ્થિર શૂન્ય read_long_string (LexState *ls, SemInfo *seminfo, size_t sep) {
  પૂર્ણાંક લીટી = ls->linenumber;  /* initial line (for error message) */
  save_and_next(ls);  /* skip 2nd '[' */
  જો (currIsNewline(ls))  /* string starts with a newline? */
    inclinenumber(ls);  /* skip it */
  માટે (;;) {
    પસંદગી (ls->current) {
      સ્થિતિ EOZ: {  /* error */
        અચલ અક્ષર *what = (seminfo ? "string" : "comment");
        અચલ અક્ષર *સંદેશ = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, લીટી);
        lexerror(ls, સંદેશ, TK_EOS);
        તોડો;  /* to avoid warnings */
      }
      સ્થિતિ ']': {
        જો (skip_sep(ls) == sep) {
          save_and_next(ls);  /* skip 2nd ']' */
          જાઓ endloop;
        }
        તોડો;
      }
      સ્થિતિ '\n': સ્થિતિ '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        જો (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        તોડો;
      }
      મૂળભૂત: {
        જો (seminfo) save_and_next(ls);
        નહીંતર next(ls);
      }
    }
  } endloop:
  જો (seminfo)
    seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + sep,
                                     luaZ_bufflen(ls->buff) - 2 * sep);
}


સ્થિર શૂન્ય esccheck (LexState *ls, પૂર્ણાંક c, અચલ અક્ષર *સંદેશ) {
  જો (!c) {
    જો (ls->current != EOZ)
      save_and_next(ls);  /* add current to buffer for error message */
    lexerror(ls, સંદેશ, TK_STRING);
  }
}


સ્થિર પૂર્ણાંક gethexa (LexState *ls) {
  save_and_next(ls);
  esccheck (ls, lisxdigit(ls->current), "hexadecimal digit expected");
  પાછા luaO_hexavalue(ls->current);
}


સ્થિર પૂર્ણાંક readhexaesc (LexState *ls) {
  પૂર્ણાંક r = gethexa(ls);
  r = (r << 4) + gethexa(ls);
  luaZ_buffremove(ls->buff, 2);  /* remove saved chars from buffer */
  પાછા r;
}


/*
** When reading a UTF-8 escape sequence, save everything to the buffer
** for error reporting in case of errors; 'i' counts the number of
** saved characters, so that they can be removed if case of success.
*/
સ્થિર l_uint32 readutf8esc (LexState *ls) {
  l_uint32 r;
  પૂર્ણાંક i = 4;  /* number of chars to be removed: start with #"\u{X" */
  save_and_next(ls);  /* skip 'u' */
  esccheck(ls, ls->current == '{', "missing '{'");
  r = cast_uint(gethexa(ls));  /* must have at least one digit */
  જ્યારે (cast_void(save_and_next(ls)), lisxdigit(ls->current)) {
    i++;
    esccheck(ls, r <= (0x7FFFFFFFu >> 4), "UTF-8 value too large");
    r = (r << 4) + luaO_hexavalue(ls->current);
  }
  esccheck(ls, ls->current == '}', "missing '}'");
  next(ls);  /* skip '}' */
  luaZ_buffremove(ls->buff, i);  /* remove saved chars from buffer */
  પાછા r;
}


સ્થિર શૂન્ય utf8esc (LexState *ls) {
  અક્ષર buff[UTF8BUFFSZ];
  પૂર્ણાંક n = luaO_utf8esc(buff, readutf8esc(ls));
  માટે (; n > 0; n--)  /* add 'buff' to string */
    save(ls, buff[UTF8BUFFSZ - n]);
}


સ્થિર પૂર્ણાંક readdecesc (LexState *ls) {
  પૂર્ણાંક i;
  પૂર્ણાંક r = 0;  /* result accumulator */
  માટે (i = 0; i < 3 && lisdigit(ls->current); i++) {  /* read up to 3 digits */
    r = 10*r + ls->current - '0';
    save_and_next(ls);
  }
  esccheck(ls, r <= UCHAR_MAX, "decimal escape too large");
  luaZ_buffremove(ls->buff, i);  /* remove read digits from buffer */
  પાછા r;
}


સ્થિર શૂન્ય read_string (LexState *ls, પૂર્ણાંક del, SemInfo *seminfo) {
  save_and_next(ls);  /* keep delimiter (for error messages) */
  જ્યારે (ls->current != del) {
    પસંદગી (ls->current) {
      સ્થિતિ EOZ:
        lexerror(ls, "unfinished string", TK_EOS);
        તોડો;  /* to avoid warnings */
      સ્થિતિ '\n':
      સ્થિતિ '\r':
        lexerror(ls, "unfinished string", TK_STRING);
        તોડો;  /* to avoid warnings */
      સ્થિતિ '\\': {  /* escape sequences */
        પૂર્ણાંક c;  /* final character to be saved */
        save_and_next(ls);  /* keep '\\' for error messages */
        પસંદગી (ls->current) {
          સ્થિતિ 'a': c = '\a'; જાઓ read_save;
          સ્થિતિ 'b': c = '\b'; જાઓ read_save;
          સ્થિતિ 'f': c = '\f'; જાઓ read_save;
          સ્થિતિ 'n': c = '\n'; જાઓ read_save;
          સ્થિતિ 'r': c = '\r'; જાઓ read_save;
          સ્થિતિ 't': c = '\t'; જાઓ read_save;
          સ્થિતિ 'v': c = '\v'; જાઓ read_save;
          સ્થિતિ 'x': c = readhexaesc(ls); જાઓ read_save;
          સ્થિતિ 'u': utf8esc(ls);  જાઓ no_save;
          સ્થિતિ '\n': સ્થિતિ '\r':
            inclinenumber(ls); c = '\n'; જાઓ only_save;
          સ્થિતિ '\\': સ્થિતિ '\"': સ્થિતિ '\'':
            c = ls->current; જાઓ read_save;
          સ્થિતિ EOZ: જાઓ no_save;  /* will raise an error next loop */
          સ્થિતિ 'z': {  /* zap following span of spaces */
            luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
            next(ls);  /* skip the 'z' */
            જ્યારે (lisspace(ls->current)) {
              જો (currIsNewline(ls)) inclinenumber(ls);
              નહીંતર next(ls);
            }
            જાઓ no_save;
          }
          મૂળભૂત: {
            esccheck(ls, lisdigit(ls->current), "invalid escape sequence");
            c = readdecesc(ls);  /* digital escape '\ddd' */
            જાઓ only_save;
          }
        }
       read_save:
         next(ls);
         /* go through */
       only_save:
         luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
         save(ls, c);
         /* go through */
       no_save: તોડો;
      }
      મૂળભૂત:
        save_and_next(ls);
    }
  }
  save_and_next(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}


સ્થિર પૂર્ણાંક llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  માટે (;;) {
    પસંદગી (ls->current) {
      સ્થિતિ '\n': સ્થિતિ '\r': {  /* line breaks */
        inclinenumber(ls);
        તોડો;
      }
      સ્થિતિ ' ': સ્થિતિ '\f': સ્થિતિ '\t': સ્થિતિ '\v': {  /* spaces */
        next(ls);
        તોડો;
      }
      સ્થિતિ '-': {  /* '-' or '--' (comment) */
        next(ls);
        જો (ls->current != '-') પાછા '-';
        /* else is a comment */
        next(ls);
        જો (ls->current == '[') {  /* long comment? */
          size_t sep = skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* 'skip_sep' may dirty the buffer */
          જો (sep >= 2) {
            read_long_string(ls, ખાલી, sep);  /* skip long comment */
            luaZ_resetbuffer(ls->buff);  /* previous call may dirty the buff. */
            તોડો;
          }
        }
        /* else short comment */
        જ્યારે (!currIsNewline(ls) && ls->current != EOZ)
          next(ls);  /* skip until end of line (or end of file) */
        તોડો;
      }
      સ્થિતિ '[': {  /* long string or simply '[' */
        size_t sep = skip_sep(ls);
        જો (sep >= 2) {
          read_long_string(ls, seminfo, sep);
          પાછા TK_STRING;
        }
        નહીંતર જો (sep == 0)  /* '[=...' missing second bracket? */
          lexerror(ls, "invalid long string delimiter", TK_STRING);
        પાછા '[';
      }
      સ્થિતિ '=': {
        next(ls);
        જો (check_next1(ls, '=')) પાછા TK_EQ;  /* '==' */
        નહીંતર પાછા '=';
      }
      સ્થિતિ '<': {
        next(ls);
        જો (check_next1(ls, '=')) પાછા TK_LE;  /* '<=' */
        નહીંતર જો (check_next1(ls, '<')) પાછા TK_SHL;  /* '<<' */
        નહીંતર પાછા '<';
      }
      સ્થિતિ '>': {
        next(ls);
        જો (check_next1(ls, '=')) પાછા TK_GE;  /* '>=' */
        નહીંતર જો (check_next1(ls, '>')) પાછા TK_SHR;  /* '>>' */
        નહીંતર પાછા '>';
      }
      સ્થિતિ '/': {
        next(ls);
        જો (check_next1(ls, '/')) પાછા TK_IDIV;  /* '//' */
        નહીંતર પાછા '/';
      }
      સ્થિતિ '~': {
        next(ls);
        જો (check_next1(ls, '=')) પાછા TK_NE;  /* '~=' */
        નહીંતર પાછા '~';
      }
      સ્થિતિ ':': {
        next(ls);
        જો (check_next1(ls, ':')) પાછા TK_DBCOLON;  /* '::' */
        નહીંતર પાછા ':';
      }
      સ્થિતિ '"': સ્થિતિ '\'': {  /* short literal strings */
        read_string(ls, ls->current, seminfo);
        પાછા TK_STRING;
      }
      સ્થિતિ '.': {  /* '.', '..', '...', or number */
        save_and_next(ls);
        જો (check_next1(ls, '.')) {
          જો (check_next1(ls, '.'))
            પાછા TK_DOTS;   /* '...' */
          નહીંતર પાછા TK_CONCAT;   /* '..' */
        }
        નહીંતર જો (!lisdigit(ls->current)) પાછા '.';
        નહીંતર પાછા read_numeral(ls, seminfo);
      }
      સ્થિતિ '0': સ્થિતિ '1': સ્થિતિ '2': સ્થિતિ '3': સ્થિતિ '4':
      સ્થિતિ '5': સ્થિતિ '6': સ્થિતિ '7': સ્થિતિ '8': સ્થિતિ '9': {
        પાછા read_numeral(ls, seminfo);
      }
      સ્થિતિ EOZ: {
        પાછા TK_EOS;
      }
      મૂળભૂત: {
        જો (lislalpha(ls->current)) {  /* identifier or reserved word? */
          TString *ts;
          કરો {
            save_and_next(ls);
          } જ્યારે (lislalnum(ls->current));
          /* find or create string */
          ts = luaS_newlstr(ls->L, luaZ_buffer(ls->buff),
                                   luaZ_bufflen(ls->buff));
          જો (isreserved(ts))   /* reserved word? */
            પાછા ts->extra - 1 + FIRST_RESERVED;
          નહીંતર {
            seminfo->ts = anchorstr(ls, ts);
            પાછા TK_NAME;
          }
        }
        નહીંતર {  /* single-char tokens ('+', '*', '%', '{', '}', ...) */
          પૂર્ણાંક c = ls->current;
          next(ls);
          પાછા c;
        }
      }
    }
  }
}


શૂન્ય luaX_next (LexState *ls) {
  ls->lastline = ls->linenumber;
  જો (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  નહીંતર
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


પૂર્ણાંક luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
  પાછા ls->lookahead.token;
}

