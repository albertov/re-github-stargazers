open Github;
open ReasonReact;

module Stargazer = {
    let component = ReasonReact.statelessComponent("Stargazer");
    let make = (~value:Github.stargazer, _children) => {
        ...component,
        render: _self =>
            <div className="stargazer item">
                <img alt="Avatar" className="ui avatar image" src=value.avatarUrl />
                <div className="content">
                    <div className="header">(string(value.login))</div>
                </div>
            </div>
    };
};

type action =
  | Init
  | GetStargazers
  | GotStargazers(fetchResponse(array(stargazer)))
  ;

type state = {
    stargazers: option(array(stargazer)),
    lastError: option(Js.Promise.error),
    loading: bool,
    curUrl: string
};

let component = ReasonReact.reducerComponent("Stargazers");

let make = (~url, ~showCount=20, ~fetchStargazers=fetchStargazers, _children) => {
    ...component,

    initialState: () => {
        stargazers: None,
        lastError: None,
        loading: false,
        curUrl: url
    },

    didMount: self => self.send(Init),

    didUpdate: ({oldSelf, newSelf}) =>
        if (oldSelf.state.curUrl != newSelf.state.curUrl) newSelf.send(GetStargazers),
    
    willReceiveProps: self => {...self.state, curUrl:url},

    reducer: action => {
        let getStargazers = self => self . ReasonReact.send(GetStargazers);
        switch (action) {
        | Init => (_state => ReasonReact.SideEffects(getStargazers));
        
        | GetStargazers => (state =>
            ReasonReact.UpdateWithSideEffects({...state, lastError:None, loading:true}, self => {
                Js.Promise.(
                       fetchStargazers(url)
                    |> then_(result => self.send(GotStargazers(result)) |> resolve)
                    |> ignore
                )
            }))

        | GotStargazers(result) => (state =>
            switch(result) {
            | Right((_,stargazers)) => {
                let count = Js.Math.min_int(showCount, Array.length(stargazers));
                let limitedStargazers = Array.sub(stargazers, 0, count);
                ReasonReact.Update({...state, loading:false, stargazers: Some(limitedStargazers)});
                }
            | Left(err) => ReasonReact.Update({...state, loading:false, lastError: Some(err)});
            });

        }
    },

    render: ({state}) => {
        <div className="ui list stargazers">
            (switch(state.stargazers) {
            | Some(gazers) => array(Array.map((value=> <Stargazer value />), gazers))
            | None => null
            })
        </div>
    }

}