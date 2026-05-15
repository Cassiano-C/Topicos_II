#!/bin/bash
# Auto-sync do projeto com merge seguro
# Autor: Cassiano

branch="main"
usuario="Cassiano"
data_hora=$(date '+%d-%m-%Y_%H-%M-%S')

# Se o usuário passar uma mensagem, usa ela
if [ -n "$1" ]; then
  mensagem="$1"
else
  mensagem="$data_hora"
fi

echo "==============================="
echo "🚀 Auto-sync iniciado - $mensagem"
echo "==============================="

# Verifica se é repositório git
if [ ! -d .git ]; then
  echo "❌ Este diretório não é um repositório Git."
  exit 1
fi

# Commit
echo "✅ Salvando alterações locais..."
git add -A
git commit -m "${mensagem} - ${usuario}" || echo "⚠️ Nenhuma alteração para commit."

# Fetch
echo "🌐 Atualizando informações do repositório remoto..."
git fetch origin

# Pull com rebase
echo "🔁 Aplicando alterações do remoto..."
if ! git pull --rebase origin "$branch"; then
    echo "⚠️ Conflitos detectados durante o rebase!"
    echo "👉 Resolva manualmente e rode o script novamente."
    exit 1
fi

# Push
echo "⬆️ Enviando alterações para o remoto..."
if git push origin "$branch"; then
    echo "✅ Tudo sincronizado com sucesso!"
else
    echo "❌ Erro ao enviar para o remoto."
    exit 1
fi

echo "==============================="
echo "✨ Sincronização concluída!"
echo "==============================="