open ReasonApolloTypes;

module MutationFactory = (Config: Config) => {
  external cast :
    string =>
    {
      .
      "data": Js.Json.t,
      "loading": bool,
    } =
    "%identity";
  [@bs.module] external gql : ReasonApolloTypes.gql = "graphql-tag";
  [@bs.module "react-apollo"]
  external mutationComponent : ReasonReact.reactClass = "Mutation";
  let graphqlMutationAST = gql(. Config.query);
  type response =
    | Loading
    | Error(apolloError)
    | Data(Config.t)
    | NotCalled;
  type renderPropObj = {
    result: response,
    data: option(Config.t),
    loading: bool,
    error: option(apolloError),
    networkStatus: int,
  };
  type renderPropObjJS = {
    .
    "loading": bool,
    "called": bool,
    "data": Js.Nullable.t(Js.Json.t),
    "error": Js.Nullable.t(apolloError),
    "networkStatus": int,
    "variables": Js.Null_undefined.t(Js.Json.t),
  };
  type optimisticResponse = Config.t;
  type mutationUpdater = (proxy, {. "data": Config.t}) => unit;
  type apolloMutation =
    (
      ~variables: Js.Json.t=?,
      ~refetchQueries: array(string)=?,
      ~update: mutationUpdater=?,
      ~optimisticResponse: optimisticResponse=?,
      unit
    ) =>
    Js.Promise.t(renderPropObjJS);
  [@bs.obj]
  external makeMutateParams :
    (
      ~variables: Js.Json.t=?,
      ~refetchQueries: array(string)=?,
      ~update: mutationUpdater=?,
      ~errorPolicy: Js.Nullable.t(string)=?,
      ~optimisticResponse: optimisticResponse=?
    ) =>
    _ =
    "";
  let apolloMutationFactory =
      (
        ~jsMutation,
        ~variables=?,
        ~refetchQueries=?,
        ~update=?,
        ~errorPolicy=?,
        ~optimisticResponse=?,
        (),
      ) =>
    jsMutation(
      makeMutateParams(
        ~variables?,
        ~refetchQueries?,
        ~update?,
        ~optimisticResponse?,
        ~errorPolicy?,
      ),
    );
  let apolloDataToReason: renderPropObjJS => response =
    apolloData =>
      switch (
        apolloData##loading,
        apolloData##data |> ReasonApolloUtils.getNonEmptyObj,
        apolloData##error |> Js.Nullable.toOption,
      ) {
      | (true, _, _) => Loading
      | (false, Some(data), None) => Data(Config.parse(data))
      | (false, _, Some(error)) => Error(error)
      | (false, None, None) => NotCalled
      };
  let convertJsInputToReason = (apolloData: renderPropObjJS) => {
    result: apolloDataToReason(apolloData),
    data:
      switch (apolloData##data |> ReasonApolloUtils.getNonEmptyObj) {
      | None => None
      | Some(data) =>
        switch (Config.parse(data)) {
        | parsedData => Some(parsedData)
        | exception _ => None
        }
      },
    error: apolloData##error |> Js.Nullable.toOption,
    loading: apolloData##loading,
    networkStatus: apolloData##networkStatus,
  };
  let make =
      (
        ~variables: option(Js.Json.t)=?,
        ~errorPolicy: option(errorPolicy)=?,
        ~update: option(mutationUpdater)=?,
        ~onError: option(apolloError => unit)=?,
        ~onCompleted: option(unit => unit)=?,
        children: (apolloMutation, renderPropObj) => ReasonReact.reactElement,
      ) =>
    ReasonReact.wrapJsForReason(
      ~reactClass=mutationComponent,
      ~props=
        Js.Nullable.(
          {
            "mutation": graphqlMutationAST,
            "update": update |> fromOption,
            "variables": variables |> fromOption,
            "onError": onError |> fromOption,
            "onCompleted": onCompleted |> fromOption,
          }
        ),
      (mutation, apolloData) =>
      children(
        apolloMutationFactory(
          ~jsMutation=mutation,
          ~errorPolicy=
            Js.Nullable.(
              switch (errorPolicy) {
              | Some(policy) =>
                switch (policy) {
                | None' => return("none")
                | All => return("all")
                | Ignore => return("ignore")
                }
              | None => undefined
              }
            ),
        ),
        convertJsInputToReason(apolloData),
      )
    );
};
