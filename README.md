# ISA 2019 - DNS Resolver #

## Společné zadání ##
***Popis:***
Vytvořte komunikující aplikaci podle konkrétní vybrané specifikace pomocí síťové knihovny BSD sockets (pokud není ve variantě zadání uvedeno jinak). Projekt bude vypracován v jazyce C/C++. Pokud individuální zadání nespecifikuje vlastní referenční systém, musí být projekt přeložitelný a spustitelný na serveru merlin.fit.vutbr.cz pod operačním systémem GNU/Linux. Program by však měl být přenositelný. Hodnocení projektů může probíhat na jiném počítači s nainstalovaným OS GNU/Linux, včetně jiných architektur než Intel/AMD, jiných distribucí, jiných verzí knihoven apod. Pokud vyžadujete minimální verzi knihovny (dostupnou na serveru merlin), jasně tuto skutečnost označte v dokumentaci a README.

Vypracovaný projekt uložený v archívu .tar a se jménem xlogin00.tar odevzdejte elektronicky přes IS. Soubor nekomprimujte.
__Termín odevzdání je 18.11.2019 (hard deadline).__ Odevzdání e-mailem po uplynutí termínu, dodatečné opravy či doplnění kódu není možné.
Odevzdaný projekt musí obsahovat:
soubor se zdrojovým kódem (dodržujte jména souborů uvedená v konkrétním zadání),
funkční Makefile pro překlad zdrojového souboru,
dokumentaci (soubor manual.pdf), která bude obsahovat uvedení do problematiky, návrhu aplikace, popis implementace, základní informace o programu, návod na použití. V dokumentaci se očekává následující: titulní strana, obsah, logické strukturování textu, přehled nastudovaných informací z literatury, popis zajímavějších pasáží implementace, použití vytvořených programů a literatura.
soubor README obsahující krátký textový popis programu s případnými rozšířeními/omezeními, příklad spuštění a seznam odevzdaných souborů,
další požadované soubory podle konkrétního typu zadání. 
Pokud v projektu nestihnete implementovat všechny požadované vlastnosti, je nutné veškerá omezení jasně uvést v dokumentaci a v souboru README.
Co není v zadání jednoznačně uvedeno, můžete implementovat podle svého vlastního výběru. Zvolené řešení popište v dokumentaci.
Při řešení projektu respektujte zvyklosti zavedené v OS unixového typu (jako je například formát textového souboru).
Vytvořené programy by měly být použitelné a smysluplné, řádně komentované a formátované a členěné do funkcí a modulů. Program by měl obsahovat nápovědu informující uživatele o činnosti programu a jeho parametrech. Případné chyby budou intuitivně popisovány uživateli.
Aplikace nesmí v žádném případě skončit s chybou SEGMENTATION FAULT ani jiným násilným systémovým ukončením (např. dělení nulou).
Pokud přejímáte krátké pasáže zdrojových kódů z různých tutoriálů či příkladů z Internetu (ne mezi sebou), tak je nutné vyznačit tyto sekce a jejich autory dle licenčních podmínek, kterými se distribuce daných zdrojových kódů řídí. V případě nedodržení bude na projekt nahlíženo jako na plagiát.
Konzultace k projektu podává vyučující, který zadání vypsal.
Před odevzdáním zkontrolujte, zda jste dodrželi všechna jména souborů požadovaná ve společné části zadání i v zadání pro konkrétní projekt. Zkontrolujte, zda je projekt přeložitelný.
***Hodnocení projektu:***
__Maximální počet bodů za projekt je 20 bodů.__
- **Maximálně 15 bodů za plně funkční aplikaci.** -
- **Maximálně 5 bodů za dokumentaci. Dokumentace se hodnotí pouze v případě funkčního kódu. Pokud kód není odevzdán nebo nefunguje podle zadání, dokumentace se nehodnotí.**
__Příklad kriterií pro hodnocení projektů:__
- nepřehledný, nekomentovaný zdrojový text: až -7 bodů
- nefunkční či chybějící Makefile: až -4 body
- nekvalitní či chybějící dokumentace: až -5 bodů
- nedodržení formátu vstupu/výstupu či konfigurace: -10 body
- odevzdaný soubor nelze přeložit, spustit a odzkoušet: 0 bodů
- odevzdáno po termínu: 0 bodů
- nedodržení zadání: 0 bodů 
- nefunkční kód: 0 bodů
- opsáno: 0 bodů (pro všechny, kdo mají stejný kód), návrh na zahájení disciplinárního řízení.

