$$ axiom \to [includes] \space funcdecl \space \{\space funcdecl\} $$
$$ includes \to \text{include} \space \{includes\}$$
$$ funcdecl \to \text{func} \space ident \space arguments \space returntype \space block$$

#### <p style="text-align: center;">func arguments</p>
$$ arguments \to () | (argumentlist) $$
$$ argumentlist \to argumentdef \space \{\space argumentlist\} $$
$$ argumentdef \to ident \space argumenttype$$
$$ argumenttype \to \space :type $$

#### <p style="text-align: center;">types</p>
$$ returntype \to \text{->} \space type $$
$$ type \to \text{i32} | \text{void} $$

#### <p style="text-align: center;">block</p>
$$ block \to \text{do} \space stmt \space \text{end}$$
$$ stmt \to variablebind|funccal \space \{stmt\}$$
$$ variablebind \to \text{bind} \space ident \space argumenttype = const$$
$$ funccal \to \text{SYS\_CALL}|\text{ident}\space callarguments $$
$$ callarguments \to (callargumentlist)  $$
$$ callargumentlist \to [const|ident \space \{\space callargumentlist\}]  $$

#### <p style="text-align: center;">indents</p>
$$ char \to a-z $$
$$ nubmer \to 0-9 $$
$$ ident \to char\{ident\}$$
$$ const \to number\{const\}$$

#### <p style="text-align: center;">keywords</p>
$$ keyword \to \text{func}|\text{include}|\text{i32}|\text{void}|\text{do}|\text{end}|\text{bind} $$
