defmodule Exdbmx do
  @on_load :load_nifs

  def load_nifs do
    :erlang.load_nif('priv/build/mdbx', 0)
  end

  # def pow(_input, _difficulty) do
  #   raise "NIF Itsuku.hash/2 not implemented"
  # end
end
