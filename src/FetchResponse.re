type fetchResponse('a) =
  | Success('a, option(string))
  | NetworkError(Js.Promise.error)
  | DecodeError(exn)
  | NotFound
  | UnexpectedStatus(int)
  ;


module Switch = {
    let component = ReasonReact.statelessComponent("FetchResponse.Switch");
    let make = ( ~value, ~successComp, ~notFoundComp=ReasonReact.null
               , ~onClickNext=None, _children
               ) => ReasonReact.({
        ...component,
        render: _self =>
            switch(value) {
            | Some(Success(items,nextPageUrl)) =>
                <div>
                    (successComp(items))
                    (switch ((onClickNext, nextPageUrl)) {
                    | (Some(action), Some(_)) =>
                        <a href="#"
                        className="link more"
                            onClick=(ev => {
                                ReactEventRe.Mouse.preventDefault(ev);
                                action();
                            })>(string("Load more"))</a>
                    | _ => null
                    })
                </div>
            | (Some(NetworkError(err))) =>
                <Message.Error
                    /* FIXME: Can we do better than stringifyAny? */
                    message=string(Js.Option.getWithDefault("", Js.Json.stringifyAny(err)))
                    header=string("There was an error when communicating with the API")
                    />
            | (Some(NotFound)) => notFoundComp
            | (Some(DecodeError(err))) =>
                <Message.Error
                    /* FIXME: Can we do better than stringifyAny? */
                    message=string(Js.Option.getWithDefault("", Js.Json.stringifyAny(err)))
                    header=string("Could not decode reponse from the API")
                    />
            | (Some(UnexpectedStatus(status))) =>
                <Message.Error
                    message=string("Code: " ++ string_of_int(status))
                    header=string("The server responded with an unexpected status code")
                    />
            | None => ReasonReact.null
            }
    });
};